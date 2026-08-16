#include "mo/Core.hpp"

#include <algorithm>
#include <array>
#include <cmath>
#include <cstring>
#include <limits>
#include <numbers>
#include <random>
#include <unordered_set>
#include <type_traits>

namespace mo {
namespace {

constexpr float kPi = std::numbers::pi_v<float>;

float clampUnit(const float value) noexcept {
    return std::clamp(value, 0.0F, 1.0F);
}

std::uint32_t variationHash(const std::uint32_t seed,
                            const int index,
                            const std::uint32_t salt) noexcept {
    std::uint32_t hash = seed ^ salt ^
                         (static_cast<std::uint32_t>(index) + 0x9E3779B9U);
    hash ^= hash >> 16U;
    hash *= 0x7FEB352DU;
    hash ^= hash >> 15U;
    hash *= 0x846CA68BU;
    hash ^= hash >> 16U;
    return hash;
}

float signedVariation(const std::uint32_t seed,
                      const int index,
                      const std::uint32_t salt) noexcept {
    return static_cast<float>(variationHash(seed, index, salt)) /
               static_cast<float>(UINT32_MAX) * 2.0F - 1.0F;
}

float effectorInfluence(const CloneInstance& instance, const EffectorParams& effector) noexcept {
    if (!effector.enabled) {
        return 0.0F;
    }
    const float deltaX = instance.x - effector.centerX;
    const float deltaY = instance.y - effector.centerY;
    float distance = 0.0F;
    switch (effector.shape) {
        case EffectorShape::Box:
            distance = std::max(std::abs(deltaX), std::abs(deltaY));
            break;
        case EffectorShape::LinearX:
            distance = std::abs(deltaX);
            break;
        case EffectorShape::LinearY:
            distance = std::abs(deltaY);
            break;
        case EffectorShape::Circle:
        default:
            distance = std::sqrt(deltaX * deltaX + deltaY * deltaY);
            break;
    }

    const float outerRadius = std::max(0.001F, effector.radius);
    const float innerRadius = std::clamp(effector.innerRadius, 0.0F, outerRadius - 0.001F);
    if (distance > outerRadius) {
        return 0.0F;
    }
    const float u = clampUnit((distance - innerRadius) / (outerRadius - innerRadius));
    const float power = std::max(0.1F, effector.power);
    float falloff = 0.0F;
    switch (effector.falloff) {
        case FalloffType::Smooth:
            falloff = 1.0F - u * u * (3.0F - 2.0F * u);
            break;
        case FalloffType::EaseIn:
            falloff = std::pow(1.0F - u, power);
            break;
        case FalloffType::EaseOut:
            falloff = 1.0F - std::pow(u, power);
            break;
        case FalloffType::Gaussian: {
            const float edge = std::exp(-power);
            falloff = (std::exp(-power * u * u) - edge) / std::max(0.0001F, 1.0F - edge);
            break;
        }
        case FalloffType::Constant:
            falloff = 1.0F;
            break;
        case FalloffType::Linear:
        default:
            falloff = 1.0F - u;
            break;
    }
    falloff = clampUnit(falloff);
    if (effector.invert) {
        falloff = 1.0F - falloff;
    }
    return falloff * clampUnit(effector.strength);
}

void applyEffector(CloneInstance& instance,
                   const EffectorParams& effector,
                   const float influence,
                   const bool secondary) noexcept {
    if (influence <= 0.0F) {
        return;
    }
    instance.x += effector.positionX * influence;
    instance.y += effector.positionY * influence;
    instance.scale *= std::max(0.001F, 1.0F + effector.scaleAmount * influence);
    instance.rotationDegrees += effector.rotationAmount * influence;
    instance.opacity += (clampUnit(effector.targetOpacity) - instance.opacity) * influence;
    if (effector.colorEnabled) {
        const float colorMix = clampUnit(effector.colorAmount * influence);
        if (secondary) {
            instance.effectorColorMix2 = colorMix;
        } else {
            instance.effectorColorMix = colorMix;
        }
    }
}

Color applyEffectorColor(const Color& base,
                         const CloneInstance& instance,
                         const EffectorParams& effector,
                         const EffectorParams& effector2) noexcept {
    Color result = base;
    const std::array<const EffectorParams*, 2> effectors{&effector, &effector2};
    const std::array<float, 2> mixes{instance.effectorColorMix, instance.effectorColorMix2};
    for (std::size_t index = 0; index < effectors.size(); ++index) {
        const float mix = clampUnit(mixes[index]);
        result.red += (clampUnit(effectors[index]->targetColor.red) - result.red) * mix;
        result.green += (clampUnit(effectors[index]->targetColor.green) - result.green) * mix;
        result.blue += (clampUnit(effectors[index]->targetColor.blue) - result.blue) * mix;
    }
    return result;
}

float animatedVariation(const RenderParams& params,
                        const int index,
                        const std::uint32_t salt) noexcept {
    if (params.variationSpeed <= 0.0F) {
        return signedVariation(params.variationSeed, index, salt);
    }
    const float phase = static_cast<float>(variationHash(params.variationSeed, index, salt ^ 0x68E31DA4U)) /
                        static_cast<float>(UINT32_MAX) * 2.0F * kPi;
    return std::sin(params.timeSeconds * params.variationSpeed * 2.0F * kPi + phase);
}

void applyWiggle(CloneInstance& instance,
                 const RenderParams& params,
                 const int index) noexcept {
    if (!params.wiggle.enabled) {
        return;
    }
    auto noise = [&](const std::uint32_t salt) {
        if (!params.wiggle.continuous || params.wiggle.speed <= 0.0F) {
            return signedVariation(params.variationSeed, index, salt);
        }
        const float sampleTime = std::max(0.0F, params.timeSeconds) * params.wiggle.speed;
        const int key = static_cast<int>(std::floor(sampleTime));
        const float fraction = sampleTime - static_cast<float>(key);
        const float smooth = fraction * fraction * (3.0F - 2.0F * fraction);
        const auto keySalt = [&](const int sample) {
            return salt ^ (static_cast<std::uint32_t>(sample) * 0x9E3779B9U);
        };
        const float from = signedVariation(params.variationSeed, index, keySalt(key));
        const float to = signedVariation(params.variationSeed, index, keySalt(key + 1));
        return from + (to - from) * smooth;
    };
    instance.x += noise(0xB5297A4DU) * params.wiggle.positionX;
    instance.y += noise(0x68E31DA4U) * params.wiggle.positionY;
    instance.scale *= std::max(0.001F, 1.0F + noise(0x1B56C4E9U) * params.wiggle.scaleAmount);
    instance.rotationDegrees += noise(0xC2B2AE35U) * params.wiggle.rotationAmount;
    instance.opacity = clampUnit(instance.opacity + noise(0x27D4EB2FU) * params.wiggle.opacityAmount);
}

void applyStep(CloneInstance& instance,
               const int index,
               const int count,
               const StepParams& step) noexcept {
    if (!step.enabled || count <= 0) {
        return;
    }
    const int orderedIndex = step.reverse ? count - 1 - index : index;
    const float threshold = static_cast<float>(orderedIndex + 1) / static_cast<float>(count);
    const float progress = clampUnit(step.progress);
    const float width = std::max(0.0001F, clampUnit(step.falloff));
    float weight = clampUnit((progress - threshold + width) / width);
    weight = weight * weight * (3.0F - 2.0F * weight);
    instance.x += step.positionX * weight;
    instance.y += step.positionY * weight;
    instance.scale *= std::max(0.001F, 1.0F + step.scaleAmount * weight);
    instance.rotationDegrees += step.rotationAmount * weight;
    instance.opacity += (clampUnit(step.targetOpacity) - instance.opacity) * weight;
}

float primitiveCoverage(const Primitive primitive,
                        const int polygonSides,
                        const float localX,
                        const float localY,
                        const float halfSize) noexcept {
    if (halfSize <= 0.0F) {
        return 0.0F;
    }

    const float x = localX / halfSize;
    const float y = localY / halfSize;
    float signedDistance = 0.0F;
    if (primitive == Primitive::Square) {
        signedDistance = std::max(std::abs(localX), std::abs(localY)) - halfSize;
    } else if (primitive == Primitive::Circle) {
        signedDistance = std::hypot(localX, localY) - halfSize;
    } else {
        const int sides = std::clamp(polygonSides, 3, 32);
        const float distance = std::sqrt(x * x + y * y);
        const float sector = 2.0F * kPi / static_cast<float>(sides);
        float angle = std::atan2(y, x) + kPi / 2.0F;
        angle = std::fmod(angle + 4.0F * kPi, sector) - sector / 2.0F;
        const float boundary = std::cos(kPi / static_cast<float>(sides)) / std::cos(angle);
        signedDistance = (distance - boundary) * halfSize;
    }
    return clampUnit(0.5F - signedDistance);
}

template <typename Component>
void blendPixel(Component* pixel,
                const PixelBuffer& image,
                const Color& color,
                const float opacity) noexcept {
    const float sourceAlpha = clampUnit(color.alpha * opacity);
    if (sourceAlpha <= 0.0F) {
        return;
    }

    const float maximum = image.componentType == ComponentType::Float32 ? 1.0F : image.integerMaximum;
    const float destinationAlpha = static_cast<float>(pixel[image.alphaIndex]) / maximum;
    const float outputAlpha = sourceAlpha + destinationAlpha * (1.0F - sourceAlpha);
    const float destinationWeight = destinationAlpha * (1.0F - sourceAlpha);
    const float inverseOutput = outputAlpha > 0.0F ? 1.0F / outputAlpha : 0.0F;

    const float source[3] = {clampUnit(color.red), clampUnit(color.green), clampUnit(color.blue)};
    const int indices[3] = {image.redIndex, image.greenIndex, image.blueIndex};
    for (int channel = 0; channel < 3; ++channel) {
        const float destination = static_cast<float>(pixel[indices[channel]]) / maximum;
        const float output = (source[channel] * sourceAlpha + destination * destinationWeight) * inverseOutput;
        if constexpr (std::is_floating_point_v<Component>) {
            pixel[indices[channel]] = clampUnit(output);
        } else {
            pixel[indices[channel]] = static_cast<Component>(std::lround(clampUnit(output) * maximum));
        }
    }
    if constexpr (std::is_floating_point_v<Component>) {
        pixel[image.alphaIndex] = clampUnit(outputAlpha);
    } else {
        pixel[image.alphaIndex] = static_cast<Component>(std::lround(clampUnit(outputAlpha) * maximum));
    }
}

std::vector<std::array<int, 2>> makeConnections(const std::vector<CloneInstance>& instances,
                                                const ConnectionParams& params) {
    constexpr std::size_t kMaximumConnections = 20000;
    constexpr std::size_t kMaximumSpatialCandidates = 2000;
    std::vector<std::array<int, 2>> result;
    if (!params.enabled || instances.size() < 2) {
        return result;
    }
    const std::size_t count = instances.size();
    const float maximumDistance = params.maxDistance > 0.0F ? params.maxDistance :
                                  std::numeric_limits<float>::max();
    auto distanceBetween = [&](const std::size_t first, const std::size_t second) {
        return std::hypot(instances[first].x - instances[second].x,
                          instances[first].y - instances[second].y);
    };
    auto add = [&](const std::size_t first, const std::size_t second) {
        if (first != second && result.size() < kMaximumConnections &&
            distanceBetween(first, second) <= maximumDistance) {
            result.push_back({static_cast<int>(first), static_cast<int>(second)});
        }
    };

    if (params.mode == ConnectionMode::Sequence) {
        for (std::size_t index = 0; index + 1 < count && result.size() < kMaximumConnections; ++index) {
            add(index, index + 1);
        }
        if (params.closeLoop && count > 2) {
            add(count - 1, 0);
        }
        return result;
    }

    const std::size_t candidateCount = std::min(count, kMaximumSpatialCandidates);
    if (params.mode == ConnectionMode::Distance) {
        for (std::size_t first = 0; first < candidateCount && result.size() < kMaximumConnections; ++first) {
            for (std::size_t second = first + 1;
                 second < candidateCount && result.size() < kMaximumConnections;
                 ++second) {
                add(first, second);
            }
        }
        return result;
    }

    const int nearestCount = std::clamp(params.connectionsPerClone, 1, 8);
    std::unordered_set<std::uint64_t> unique;
    for (std::size_t first = 0; first < candidateCount && result.size() < kMaximumConnections; ++first) {
        std::vector<std::pair<float, int>> distances;
        distances.reserve(candidateCount - 1);
        for (std::size_t second = 0; second < candidateCount; ++second) {
            if (first != second) {
                const float distance = distanceBetween(first, second);
                if (distance <= maximumDistance) {
                    distances.emplace_back(distance, static_cast<int>(second));
                }
            }
        }
        const std::size_t selected = std::min<std::size_t>(distances.size(),
                                                            static_cast<std::size_t>(nearestCount));
        std::partial_sort(distances.begin(), distances.begin() + selected, distances.end());
        for (std::size_t nearest = 0; nearest < selected; ++nearest) {
            const int second = distances[nearest].second;
            const auto low = static_cast<std::uint32_t>(std::min<int>(static_cast<int>(first), second));
            const auto high = static_cast<std::uint32_t>(std::max<int>(static_cast<int>(first), second));
            const std::uint64_t key = (static_cast<std::uint64_t>(low) << 32U) | high;
            if (unique.insert(key).second) {
                result.push_back({static_cast<int>(low), static_cast<int>(high)});
            }
        }
    }
    return result;
}

template <typename Component>
void renderConnectionsTyped(const PixelBuffer& image,
                            const RenderParams& params,
                            const std::vector<CloneInstance>& instances) {
    const auto connections = makeConnections(instances, params.connections);
    for (const auto& connection : connections) {
        const CloneInstance& first = instances[static_cast<std::size_t>(connection[0])];
        const CloneInstance& second = instances[static_cast<std::size_t>(connection[1])];
        float thickness = std::max(0.1F, params.connections.thickness);
        float opacity = clampUnit(params.connections.opacity);
        if (params.lineEffector.enabled) {
            CloneInstance midpoint{};
            midpoint.x = (first.x + second.x) * 0.5F;
            midpoint.y = (first.y + second.y) * 0.5F;
            const float influence = effectorInfluence(midpoint, params.lineEffector);
            opacity += (clampUnit(params.lineEffector.targetOpacity) - opacity) * influence;
            thickness *= std::max(0.01F, 1.0F + params.lineEffector.scaleAmount * influence);
        }
        if (opacity <= 0.0F || thickness <= 0.0F) {
            continue;
        }

        const float firstX = first.x - static_cast<float>(image.originX);
        const float firstY = first.y - static_cast<float>(image.originY);
        const float secondX = second.x - static_cast<float>(image.originX);
        const float secondY = second.y - static_cast<float>(image.originY);
        const float deltaX = secondX - firstX;
        const float deltaY = secondY - firstY;
        const float lengthSquared = std::max(0.0001F, deltaX * deltaX + deltaY * deltaY);
        const float padding = thickness * 0.5F + 1.0F;
        const int minX = std::max(0, static_cast<int>(std::floor(std::min(firstX, secondX) - padding)));
        const int maxX = std::min(image.width - 1,
                                  static_cast<int>(std::ceil(std::max(firstX, secondX) + padding)));
        const int minY = std::max(0, static_cast<int>(std::floor(std::min(firstY, secondY) - padding)));
        const int maxY = std::min(image.height - 1,
                                  static_cast<int>(std::ceil(std::max(firstY, secondY) + padding)));
        for (int y = minY; y <= maxY; ++y) {
            auto* row = reinterpret_cast<Component*>(
                static_cast<std::uint8_t*>(image.pixels) + static_cast<std::ptrdiff_t>(y) * image.rowBytes);
            for (int x = minX; x <= maxX; ++x) {
                const float pixelX = static_cast<float>(x) + 0.5F;
                const float pixelY = static_cast<float>(y) + 0.5F;
                const float projection = clampUnit(((pixelX - firstX) * deltaX +
                                                    (pixelY - firstY) * deltaY) / lengthSquared);
                const float closestX = firstX + projection * deltaX;
                const float closestY = firstY + projection * deltaY;
                const float distance = std::hypot(pixelX - closestX, pixelY - closestY);
                const float coverage = clampUnit(thickness * 0.5F + 0.5F - distance);
                if (coverage > 0.0F) {
                    blendPixel(row + static_cast<std::ptrdiff_t>(x) * 4,
                               image,
                               params.connections.color,
                               opacity * coverage);
                }
            }
        }
    }
}

template <typename Component>
void renderTyped(const PixelBuffer& image, const RenderParams& params) {
    const auto instances = makeInstances(params);
    renderConnectionsTyped<Component>(image, params, instances);
    for (const CloneInstance& instance : instances) {
        const float size = std::max(0.0F, params.cloneSize * instance.scale);
        const float halfSize = size * 0.5F;
        const float localCenterX = instance.x - static_cast<float>(image.originX);
        const float localCenterY = instance.y - static_cast<float>(image.originY);
        const int minX = std::max(0, static_cast<int>(std::floor(localCenterX - halfSize - 1.0F)));
        const int maxX = std::min(image.width - 1, static_cast<int>(std::ceil(localCenterX + halfSize + 1.0F)));
        const int minY = std::max(0, static_cast<int>(std::floor(localCenterY - halfSize - 1.0F)));
        const int maxY = std::min(image.height - 1, static_cast<int>(std::ceil(localCenterY + halfSize + 1.0F)));
        const float radians = instance.rotationDegrees * kPi / 180.0F;
        const float cosine = std::cos(radians);
        const float sine = std::sin(radians);

        for (int y = minY; y <= maxY; ++y) {
            auto* row = reinterpret_cast<Component*>(
                static_cast<std::uint8_t*>(image.pixels) + static_cast<std::ptrdiff_t>(y) * image.rowBytes);
            for (int x = minX; x <= maxX; ++x) {
                const float deltaX = static_cast<float>(x) + 0.5F - localCenterX;
                const float deltaY = static_cast<float>(y) + 0.5F - localCenterY;
                const float localX = deltaX * cosine + deltaY * sine;
                const float localY = -deltaX * sine + deltaY * cosine;
                const float coverage = primitiveCoverage(params.primitive,
                                                         params.polygonSides,
                                                         localX,
                                                         localY,
                                                         halfSize);
                if (coverage > 0.0F) {
                    const Color& baseColor = params.paletteEnabled ?
                        params.palette[static_cast<std::size_t>(instance.colorIndex)] : params.color;
                    const Color color = applyEffectorColor(baseColor, instance, params.effector, params.effector2);
                    blendPixel(row + static_cast<std::ptrdiff_t>(x) * 4,
                               image,
                               color,
                               instance.opacity * coverage);
                }
            }
        }
    }
}

template <typename Component>
void renderSourcesTyped(const PixelBuffer& image,
                        const std::vector<PixelBuffer>& sources,
                        const RenderParams& params) {
    struct SourceBounds {
        PixelBuffer image{};
        int left{};
        int top{};
        int right{};
        int bottom{};
    };
    std::vector<SourceBounds> validSources;
    validSources.reserve(sources.size());
    for (const PixelBuffer& source : sources) {
        if (!source.pixels || source.componentType != image.componentType) {
            continue;
        }
        const float maximum = source.componentType == ComponentType::Float32 ? 1.0F : source.integerMaximum;
        SourceBounds bounds{source, source.width, source.height, -1, -1};
        for (int y = 0; y < source.height; ++y) {
            const auto* row = reinterpret_cast<const Component*>(
                static_cast<const std::uint8_t*>(source.pixels) + static_cast<std::ptrdiff_t>(y) * source.rowBytes);
            for (int x = 0; x < source.width; ++x) {
                const auto* pixel = row + static_cast<std::ptrdiff_t>(x) * 4;
                if (static_cast<float>(pixel[source.alphaIndex]) > maximum * 0.0001F) {
                    bounds.left = std::min(bounds.left, x);
                    bounds.top = std::min(bounds.top, y);
                    bounds.right = std::max(bounds.right, x);
                    bounds.bottom = std::max(bounds.bottom, y);
                }
            }
        }
        if (bounds.right >= bounds.left && bounds.bottom >= bounds.top) {
            validSources.push_back(bounds);
        }
    }
    if (validSources.empty()) {
        return;
    }

    const auto instances = makeInstances(params);
    renderConnectionsTyped<Component>(image, params, instances);
    for (const CloneInstance& instance : instances) {
        std::size_t sourceIndex = static_cast<std::size_t>(instance.index) % validSources.size();
        if (params.sourceSelection == SourceSelection::Random) {
            std::uint32_t hash = params.sourceRandomSeed ^
                                 (static_cast<std::uint32_t>(instance.index) + 0x9E3779B9U);
            hash ^= hash >> 16U;
            hash *= 0x7FEB352DU;
            hash ^= hash >> 15U;
            hash *= 0x846CA68BU;
            hash ^= hash >> 16U;
            sourceIndex = static_cast<std::size_t>(hash) % validSources.size();
        }
        const SourceBounds& bounds = validSources[sourceIndex];
        const PixelBuffer& source = bounds.image;
        const float sourceMaximum = source.componentType == ComponentType::Float32 ? 1.0F : source.integerMaximum;
        const float sourceWidth = static_cast<float>(bounds.right - bounds.left + 1);
        const float sourceHeight = static_cast<float>(bounds.bottom - bounds.top + 1);
        const float sourceCenterX = (static_cast<float>(bounds.left) + static_cast<float>(bounds.right) + 1.0F) * 0.5F;
        const float sourceCenterY = (static_cast<float>(bounds.top) + static_cast<float>(bounds.bottom) + 1.0F) * 0.5F;
        const float fitDimension = std::max(sourceWidth, sourceHeight);
        const float scale = std::max(0.0001F, params.cloneSize * instance.scale / fitDimension);
        const float halfWidth = sourceWidth * scale * 0.5F;
        const float halfHeight = sourceHeight * scale * 0.5F;
        const float boundsRadius = std::hypot(halfWidth, halfHeight);
        const float localCenterX = instance.x - static_cast<float>(image.originX);
        const float localCenterY = instance.y - static_cast<float>(image.originY);
        const int minX = std::max(0, static_cast<int>(std::floor(localCenterX - boundsRadius - 1.0F)));
        const int maxX = std::min(image.width - 1, static_cast<int>(std::ceil(localCenterX + boundsRadius + 1.0F)));
        const int minY = std::max(0, static_cast<int>(std::floor(localCenterY - boundsRadius - 1.0F)));
        const int maxY = std::min(image.height - 1, static_cast<int>(std::ceil(localCenterY + boundsRadius + 1.0F)));
        const float radians = instance.rotationDegrees * kPi / 180.0F;
        const float cosine = std::cos(radians);
        const float sine = std::sin(radians);

        for (int y = minY; y <= maxY; ++y) {
            auto* outputRow = reinterpret_cast<Component*>(
                static_cast<std::uint8_t*>(image.pixels) + static_cast<std::ptrdiff_t>(y) * image.rowBytes);
            for (int x = minX; x <= maxX; ++x) {
                const float deltaX = static_cast<float>(x) + 0.5F - localCenterX;
                const float deltaY = static_cast<float>(y) + 0.5F - localCenterY;
                const float unrotatedX = deltaX * cosine + deltaY * sine;
                const float unrotatedY = -deltaX * sine + deltaY * cosine;
                const int sourceX = static_cast<int>(std::floor(sourceCenterX + unrotatedX / scale));
                const int sourceY = static_cast<int>(std::floor(sourceCenterY + unrotatedY / scale));
                if (sourceX < bounds.left || sourceX > bounds.right ||
                    sourceY < bounds.top || sourceY > bounds.bottom) {
                    continue;
                }
                const auto* sourceRow = reinterpret_cast<const Component*>(
                    static_cast<const std::uint8_t*>(source.pixels) +
                    static_cast<std::ptrdiff_t>(sourceY) * source.rowBytes);
                const auto* sourcePixel = sourceRow + static_cast<std::ptrdiff_t>(sourceX) * 4;
                const float alpha = clampUnit(static_cast<float>(sourcePixel[source.alphaIndex]) / sourceMaximum);
                if (alpha <= 0.0F) {
                    continue;
                }
                Color color{};
                color.alpha = alpha;
                color.red = clampUnit(static_cast<float>(sourcePixel[source.redIndex]) / sourceMaximum / alpha);
                color.green = clampUnit(static_cast<float>(sourcePixel[source.greenIndex]) / sourceMaximum / alpha);
                color.blue = clampUnit(static_cast<float>(sourcePixel[source.blueIndex]) / sourceMaximum / alpha);
                if (params.paletteEnabled && params.paletteTintsSources) {
                    const Color& tint = params.palette[static_cast<std::size_t>(instance.colorIndex)];
                    color.red *= clampUnit(tint.red);
                    color.green *= clampUnit(tint.green);
                    color.blue *= clampUnit(tint.blue);
                }
                color = applyEffectorColor(color, instance, params.effector, params.effector2);
                blendPixel(outputRow + static_cast<std::ptrdiff_t>(x) * 4,
                           image,
                           color,
                           instance.opacity);
            }
        }
    }
}

}  // namespace

std::vector<CloneInstance> makeInstances(const RenderParams& params) {
    const int count = std::clamp(params.cloneCount, 0, 100000);
    std::vector<CloneInstance> instances;
    instances.reserve(static_cast<std::size_t>(count));
    if (count == 0) {
        return instances;
    }

    const int columns = std::max(1, params.columns);
    const int rows = std::max(1, params.rows);
    std::mt19937 random(params.randomSeed);
    std::uniform_real_distribution<float> randomX(-0.5F, 0.5F);
    std::uniform_real_distribution<float> randomY(-0.5F, 0.5F);

    constexpr int pathSamples = 128;
    std::array<float, pathSamples + 1> pathLengths{};
    float totalPathLength = 0.0F;
    auto pathPoint = [&](const float t) {
        const float inverse = 1.0F - t;
        const float a = inverse * inverse * inverse;
        const float b = 3.0F * inverse * inverse * t;
        const float c = 3.0F * inverse * t * t;
        const float d = t * t * t;
        return std::array<float, 2>{
            a * params.pathStartX + b * params.pathControl1X + c * params.pathControl2X + d * params.pathEndX,
            a * params.pathStartY + b * params.pathControl1Y + c * params.pathControl2Y + d * params.pathEndY};
    };
    if (params.formation == Formation::Path) {
        auto previous = pathPoint(0.0F);
        for (int sample = 1; sample <= pathSamples; ++sample) {
            const auto point = pathPoint(static_cast<float>(sample) / static_cast<float>(pathSamples));
            totalPathLength += std::hypot(point[0] - previous[0], point[1] - previous[1]);
            pathLengths[static_cast<std::size_t>(sample)] = totalPathLength;
            previous = point;
        }
    }

    for (int index = 0; index < count; ++index) {
        CloneInstance instance{};
        instance.index = index;
        const int paletteSize = std::clamp(params.paletteSize, 1, 4);
        instance.colorIndex = params.paletteSelection == SourceSelection::Random ?
            static_cast<int>(variationHash(params.paletteSeed, index, 0xD1B54A35U) %
                             static_cast<std::uint32_t>(paletteSize)) :
            index % paletteSize;
        instance.rotationDegrees = params.rotationDegrees;
        instance.opacity = clampUnit(params.opacity);

        switch (params.formation) {
            case Formation::ZCircle: {
                const float orbitRotation = params.zCircleRotation +
                                            params.zCircleSpeed * params.timeSeconds;
                const float angle = -kPi / 2.0F + orbitRotation * kPi / 180.0F +
                                    2.0F * kPi * static_cast<float>(index) /
                                        static_cast<float>(count);
                const float tilt = params.zCircleTilt * kPi / 180.0F;
                const float x = std::cos(angle);
                const float ringDepth = std::sin(angle);
                instance.x = params.centerX + x * params.radius;
                instance.y = params.centerY + ringDepth * std::sin(tilt) * params.radius;
                instance.depth = clampUnit((ringDepth * std::cos(tilt) + 1.0F) * 0.5F);
                const float depthAmount = clampUnit(params.zCircleDepthScale);
                instance.scale *= (1.0F - depthAmount) + depthAmount * instance.depth;
                break;
            }
            case Formation::Path: {
                const float baseNormalized = count == 1 ? 0.5F :
                    static_cast<float>(index) / static_cast<float>(count - 1);
                float normalized = baseNormalized;
                if (std::abs(params.pathPositionOffset) > 0.000001F) {
                    const float shifted = baseNormalized + params.pathPositionOffset;
                    normalized = shifted - std::floor(shifted);
                }
                const float targetLength = normalized * totalPathLength;
                const auto upper = std::lower_bound(pathLengths.begin(), pathLengths.end(), targetLength);
                const int upperIndex = std::clamp(static_cast<int>(upper - pathLengths.begin()), 1, pathSamples);
                const float lowerLength = pathLengths[static_cast<std::size_t>(upperIndex - 1)];
                const float upperLength = pathLengths[static_cast<std::size_t>(upperIndex)];
                const float segment = upperLength > lowerLength ?
                    (targetLength - lowerLength) / (upperLength - lowerLength) : 0.0F;
                const float t = (static_cast<float>(upperIndex - 1) + segment) /
                                static_cast<float>(pathSamples);
                const auto point = pathPoint(t);
                instance.x = point[0];
                instance.y = point[1];
                if (params.pathAlign) {
                    const float inverse = 1.0F - t;
                    const float tangentX = 3.0F * inverse * inverse *
                                               (params.pathControl1X - params.pathStartX) +
                                           6.0F * inverse * t *
                                               (params.pathControl2X - params.pathControl1X) +
                                           3.0F * t * t * (params.pathEndX - params.pathControl2X);
                    const float tangentY = 3.0F * inverse * inverse *
                                               (params.pathControl1Y - params.pathStartY) +
                                           6.0F * inverse * t *
                                               (params.pathControl2Y - params.pathControl1Y) +
                                           3.0F * t * t * (params.pathEndY - params.pathControl2Y);
                    instance.rotationDegrees += std::atan2(tangentY, tangentX) * 180.0F / kPi +
                                                params.pathRotationOffset;
                }
                break;
            }
            case Formation::Globe: {
                constexpr float goldenAngle = kPi * (3.0F - 2.2360679774997896964F);
                const float normalizedY = 1.0F - 2.0F *
                    (static_cast<float>(index) + 0.5F) / static_cast<float>(count);
                const float ringRadius = std::sqrt(std::max(0.0F, 1.0F - normalizedY * normalizedY));
                const float longitude = goldenAngle * static_cast<float>(index);
                float x = std::cos(longitude) * ringRadius;
                float y = normalizedY;
                float z = std::sin(longitude) * ringRadius;

                const float rotationX = params.globeRotationX * kPi / 180.0F;
                const float rotationY = params.globeRotationY * kPi / 180.0F;
                const float rotationZ = params.globeRotationZ * kPi / 180.0F;
                const float cosX = std::cos(rotationX);
                const float sinX = std::sin(rotationX);
                const float cosY = std::cos(rotationY);
                const float sinY = std::sin(rotationY);
                const float cosZ = std::cos(rotationZ);
                const float sinZ = std::sin(rotationZ);

                const float yAfterX = y * cosX - z * sinX;
                const float zAfterX = y * sinX + z * cosX;
                const float xAfterY = x * cosY + zAfterX * sinY;
                const float zAfterY = -x * sinY + zAfterX * cosY;
                x = xAfterY * cosZ - yAfterX * sinZ;
                y = xAfterY * sinZ + yAfterX * cosZ;
                z = zAfterY;

                instance.x = params.centerX + x * params.radius;
                instance.y = params.centerY + y * params.radius;
                instance.depth = clampUnit((z + 1.0F) * 0.5F);
                const float depthAmount = clampUnit(params.globeDepthScale);
                instance.scale *= (1.0F - depthAmount) + depthAmount * instance.depth;
                break;
            }
            case Formation::Linear: {
                const float offset = static_cast<float>(index) - static_cast<float>(count - 1) * 0.5F;
                instance.x = params.centerX + offset * params.spacingX;
                instance.y = params.centerY;
                break;
            }
            case Formation::Circle: {
                const int rings = std::clamp(params.circleRings, 1, count);
                const int totalWeight = rings * (rings + 1) / 2;
                const int extraClones = count - rings;
                int ring = 0;
                int ringStart = 0;
                int ringEnd = 1;
                for (; ring < rings; ++ring) {
                    const int previousWeight = ring * (ring + 1) / 2;
                    const int cumulativeWeight = (ring + 1) * (ring + 2) / 2;
                    ringStart = ring + extraClones * previousWeight / totalWeight;
                    ringEnd = ring + 1 + extraClones * cumulativeWeight / totalWeight;
                    if (index < ringEnd) {
                        break;
                    }
                }
                ring = std::min(ring, rings - 1);
                const int clonesOnRing = std::max(1, ringEnd - ringStart);
                const int localIndex = index - ringStart;
                const float stagger = (ring % 2 == 0) ? 0.0F : kPi / static_cast<float>(clonesOnRing);
                const float angle = -kPi / 2.0F + stagger +
                                    2.0F * kPi * static_cast<float>(localIndex) /
                                        static_cast<float>(clonesOnRing);
                const float ringRadius = params.radius * static_cast<float>(ring + 1) /
                                         static_cast<float>(rings);
                instance.x = params.centerX + std::cos(angle) * ringRadius;
                instance.y = params.centerY + std::sin(angle) * ringRadius;
                break;
            }
            case Formation::Scatter: {
                instance.x = params.centerX + randomX(random) * params.spacingX * static_cast<float>(columns);
                instance.y = params.centerY + randomY(random) * params.spacingY * static_cast<float>(rows);
                break;
            }
            case Formation::Grid:
            default: {
                const int row = index / columns;
                const int column = index % columns;
                const int usedRows = std::max(1, (count + columns - 1) / columns);
                instance.x = params.centerX +
                             (static_cast<float>(column) - static_cast<float>(columns - 1) * 0.5F) *
                                 params.spacingX;
                instance.y = params.centerY +
                             (static_cast<float>(row) - static_cast<float>(usedRows - 1) * 0.5F) *
                                 params.spacingY;
                break;
            }
        }
        if (params.formationRotation != 0.0F) {
            const float radians = params.formationRotation * kPi / 180.0F;
            const float cosine = std::cos(radians);
            const float sine = std::sin(radians);
            const float deltaX = instance.x - params.centerX;
            const float deltaY = instance.y - params.centerY;
            instance.x = params.centerX + deltaX * cosine - deltaY * sine;
            instance.y = params.centerY + deltaX * sine + deltaY * cosine;
            instance.rotationDegrees += params.formationRotation;
        }
        instance.x += animatedVariation(params, index, 0xA341316CU) * params.positionJitterX;
        instance.y += animatedVariation(params, index, 0xC8013EA4U) * params.positionJitterY;
        instance.scale *= std::max(0.001F,
                                   1.0F + animatedVariation(params, index, 0xAD90777DU) *
                                               params.scaleJitter);
        instance.rotationDegrees += animatedVariation(params, index, 0x7E95761EU) *
                                    params.rotationJitter;
        instance.opacity = clampUnit(instance.opacity +
                                     animatedVariation(params, index, 0x4B7A70E9U) *
                                         params.opacityJitter);
        instance.scale *= std::max(0.001F,
                                   1.0F + signedVariation(params.variationSeed, index, 0xF1357AE1U) *
                                               params.appearanceRandomSize);
        applyWiggle(instance, params, index);
        applyStep(instance, index, count, params.step);
        const float primaryInfluence = effectorInfluence(instance, params.effector);
        const float secondaryInfluence = effectorInfluence(instance, params.effector2);
        applyEffector(instance, params.effector, primaryInfluence, false);
        applyEffector(instance, params.effector2, secondaryInfluence, true);
        instances.push_back(instance);
    }
    if (params.formation == Formation::Globe || params.formation == Formation::ZCircle) {
        std::stable_sort(instances.begin(), instances.end(), [](const CloneInstance& left,
                                                                const CloneInstance& right) {
            return left.depth < right.depth;
        });
    }
    return instances;
}

void clear(const ImageView image) noexcept {
    if (!image.pixels || image.width <= 0 || image.height <= 0 || image.rowBytes <= 0) {
        return;
    }
    for (int y = 0; y < image.height; ++y) {
        std::memset(image.pixels + static_cast<std::ptrdiff_t>(y) * image.rowBytes,
                    0,
                    static_cast<std::size_t>(image.width) * 4U);
    }
}

void render(const ImageView image, const RenderParams& params) {
    if (!image.pixels || image.width <= 0 || image.height <= 0 || image.rowBytes < image.width * 4) {
        return;
    }
    render(PixelBuffer{image.pixels,
                       image.width,
                       image.height,
                       image.rowBytes,
                       0,
                       0,
                       ComponentType::UInt8,
                       3,
                       0,
                       1,
                       2,
                       255.0F},
           params);
}

void clear(const PixelBuffer image) noexcept {
    if (!image.pixels || image.width <= 0 || image.height <= 0 || image.rowBytes <= 0) {
        return;
    }
    const std::size_t componentBytes = image.componentType == ComponentType::UInt8 ? 1U :
                                       image.componentType == ComponentType::UInt16 ? 2U : 4U;
    for (int y = 0; y < image.height; ++y) {
        std::memset(static_cast<std::uint8_t*>(image.pixels) + static_cast<std::ptrdiff_t>(y) * image.rowBytes,
                    0,
                    static_cast<std::size_t>(image.width) * 4U * componentBytes);
    }
}

void render(const PixelBuffer image, const RenderParams& params) {
    if (!image.pixels || image.width <= 0 || image.height <= 0 || image.rowBytes <= 0) {
        return;
    }
    switch (image.componentType) {
        case ComponentType::UInt16:
            renderTyped<std::uint16_t>(image, params);
            break;
        case ComponentType::Float32:
            renderTyped<float>(image, params);
            break;
        case ComponentType::UInt8:
        default:
            renderTyped<std::uint8_t>(image, params);
            break;
    }
}

void renderSource(const PixelBuffer image, const PixelBuffer source, const RenderParams& params) {
    renderSources(image, std::vector<PixelBuffer>{source}, params);
}

void renderSources(const PixelBuffer image,
                   const std::vector<PixelBuffer>& sources,
                   const RenderParams& params) {
    if (!image.pixels || sources.empty() || image.width <= 0 || image.height <= 0) {
        return;
    }
    switch (image.componentType) {
        case ComponentType::UInt16:
            renderSourcesTyped<std::uint16_t>(image, sources, params);
            break;
        case ComponentType::Float32:
            renderSourcesTyped<float>(image, sources, params);
            break;
        case ComponentType::UInt8:
        default:
            renderSourcesTyped<std::uint8_t>(image, sources, params);
            break;
    }
}

}  // namespace mo

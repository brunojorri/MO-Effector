#include "mo/Core.hpp"

#include <algorithm>
#include <cmath>
#include <cstdint>
#include <iostream>
#include <stdexcept>
#include <vector>

namespace {

void require(const bool condition, const char* message) {
    if (!condition) {
        throw std::runtime_error(message);
    }
}

bool near(const float left, const float right, const float tolerance = 0.001F) {
    return std::abs(left - right) <= tolerance;
}

void testGridIsCentered() {
    mo::RenderParams params{};
    params.formation = mo::Formation::Grid;
    params.cloneCount = 6;
    params.columns = 3;
    params.centerX = 100.0F;
    params.centerY = 50.0F;
    params.spacingX = 10.0F;
    params.spacingY = 20.0F;

    const auto clones = mo::makeInstances(params);
    require(clones.size() == 6U, "Grid must create every requested clone.");
    require(near(clones.front().x, 90.0F) && near(clones.front().y, 40.0F),
            "Grid first clone is not centered correctly.");
    require(near(clones.back().x, 110.0F) && near(clones.back().y, 60.0F),
            "Grid last clone is not centered correctly.");
}

void testCircleClosesWithoutDuplicateEndpoint() {
    mo::RenderParams params{};
    params.formation = mo::Formation::Circle;
    params.cloneCount = 4;
    params.centerX = 100.0F;
    params.centerY = 100.0F;
    params.radius = 25.0F;

    const auto clones = mo::makeInstances(params);
    require(near(clones[0].x, 100.0F) && near(clones[0].y, 75.0F), "Circle must begin at the top.");
    require(near(clones[1].x, 125.0F) && near(clones[1].y, 100.0F), "Circle quarter turn is wrong.");
    require(!near(clones.front().x, clones.back().x) || !near(clones.front().y, clones.back().y),
            "Circle duplicated its endpoint.");
}

void testScatterIsDeterministic() {
    mo::RenderParams params{};
    params.formation = mo::Formation::Scatter;
    params.cloneCount = 20;
    params.randomSeed = 42;
    const auto first = mo::makeInstances(params);
    const auto second = mo::makeInstances(params);
    require(first.size() == second.size(), "Scatter sizes differ.");
    for (std::size_t index = 0; index < first.size(); ++index) {
        require(near(first[index].x, second[index].x) && near(first[index].y, second[index].y),
                "Scatter seed is not deterministic.");
    }
}

void testRendererWritesOneBuffer() {
    constexpr int width = 128;
    constexpr int height = 128;
    std::vector<std::uint8_t> pixels(static_cast<std::size_t>(width * height * 4), 0);
    mo::ImageView image{pixels.data(), width, height, width * 4};
    mo::RenderParams params{};
    params.formation = mo::Formation::Grid;
    params.primitive = mo::Primitive::Circle;
    params.cloneCount = 4;
    params.columns = 2;
    params.centerX = 64.0F;
    params.centerY = 64.0F;
    params.spacingX = 40.0F;
    params.spacingY = 40.0F;
    params.cloneSize = 16.0F;

    mo::clear(image);
    mo::render(image, params);
    const auto visiblePixels = std::count_if(pixels.begin() + 3, pixels.end(), [offset = 0](std::uint8_t value) mutable {
        const bool alpha = offset % 4 == 0;
        ++offset;
        return alpha && value > 0;
    });
    require(visiblePixels > 400, "Renderer produced no useful visible result.");
    require(visiblePixels < 1200, "Renderer wrote far beyond the expected clone area.");
}

void testInternalCircleHasAntialiasedEdges() {
    constexpr int size = 128;
    std::vector<std::uint8_t> pixels(static_cast<std::size_t>(size * size * 4), 0);
    mo::ImageView image{pixels.data(), size, size, size * 4};
    mo::RenderParams params{};
    params.primitive = mo::Primitive::Circle;
    params.cloneCount = 1;
    params.columns = 1;
    params.centerX = 64.0F;
    params.centerY = 64.0F;
    params.cloneSize = 51.0F;
    mo::clear(image);
    mo::render(image, params);
    bool foundPartialCoverage = false;
    for (std::size_t offset = 3; offset < pixels.size(); offset += 4) {
        foundPartialCoverage = foundPartialCoverage ||
                               (pixels[offset] > 0 && pixels[offset] < 255);
    }
    require(foundPartialCoverage, "Circle edge was rendered without antialiasing coverage.");
}

void testEffectorUsesItsOwnCenterAndRadius() {
    mo::RenderParams params{};
    params.formation = mo::Formation::Linear;
    params.cloneCount = 1;
    params.centerX = 200.0F;
    params.centerY = 100.0F;
    params.effector.enabled = true;
    params.effector.centerX = 200.0F;
    params.effector.centerY = 100.0F;
    params.effector.radius = 80.0F;
    params.effector.innerRadius = 0.0F;
    params.effector.positionX = 50.0F;
    params.effector.positionY = 0.0F;
    params.effector.scaleAmount = 0.0F;
    params.effector.rotationAmount = 0.0F;
    params.effector.targetOpacity = 1.0F;
    params.effector.colorEnabled = true;
    params.effector.colorAmount = 1.0F;

    const auto affected = mo::makeInstances(params);
    require(near(affected[0].x, 250.0F), "Effector did not reach maximum strength at its center.");
    require(near(affected[0].effectorColorMix, 1.0F),
            "Effector color did not reach full strength at its center.");

    params.effector.centerX = 500.0F;
    const auto outside = mo::makeInstances(params);
    require(near(outside[0].x, 200.0F), "Effector changed a clone outside its radius.");
    require(near(outside[0].effectorColorMix, 0.0F),
            "Effector color changed a clone outside its radius.");
}

void testGlobeProjectsDepthAndSortsBackToFront() {
    mo::RenderParams params{};
    params.formation = mo::Formation::Globe;
    params.cloneCount = 40;
    params.centerX = 100.0F;
    params.centerY = 100.0F;
    params.radius = 60.0F;
    params.globeDepthScale = 0.8F;
    const auto clones = mo::makeInstances(params);
    require(clones.size() == 40U, "Globe must preserve clone count.");
    for (std::size_t index = 1; index < clones.size(); ++index) {
        require(clones[index - 1].depth <= clones[index].depth,
                "Globe instances must render back-to-front.");
    }
    require(clones.front().scale < clones.back().scale,
            "Globe depth must change the apparent clone scale.");
    for (const auto& clone : clones) {
        const float distance = std::hypot(clone.x - params.centerX, clone.y - params.centerY);
        require(distance <= params.radius + 0.01F, "Globe projection escaped its radius.");
    }
}

void testSourceLayerPixelsAreCloned() {
    constexpr int sourceSize = 8;
    constexpr int outputSize = 64;
    std::vector<std::uint8_t> source(static_cast<std::size_t>(sourceSize * sourceSize * 4), 0);
    for (int y = 2; y < 6; ++y) {
        for (int x = 2; x < 6; ++x) {
            const std::size_t offset = static_cast<std::size_t>((y * sourceSize + x) * 4);
            source[offset] = 255;
            source[offset + 1] = 255;
        }
    }
    std::vector<std::uint8_t> output(static_cast<std::size_t>(outputSize * outputSize * 4), 0);
    mo::PixelBuffer sourceImage{source.data(), sourceSize, sourceSize, sourceSize * 4,
                                0, 0, mo::ComponentType::UInt8, 0, 1, 2, 3, 255.0F};
    mo::PixelBuffer outputImage{output.data(), outputSize, outputSize, outputSize * 4,
                                0, 0, mo::ComponentType::UInt8, 0, 1, 2, 3, 255.0F};
    mo::RenderParams params{};
    params.formation = mo::Formation::Linear;
    params.cloneCount = 1;
    params.columns = 1;
    params.centerX = 32.0F;
    params.centerY = 32.0F;
    params.cloneSize = 20.0F;
    mo::clear(outputImage);
    mo::renderSource(outputImage, sourceImage, params);
    const std::size_t center = static_cast<std::size_t>((32 * outputSize + 32) * 4);
    require(output[center] == 255 && output[center + 1] == 255,
            "Source layer color and alpha were not cloned.");
}

void testMultiSourceCyclesByCloneIndex() {
    constexpr int sourceSize = 4;
    constexpr int outputSize = 64;
    std::vector<std::uint8_t> red(static_cast<std::size_t>(sourceSize * sourceSize * 4), 0);
    std::vector<std::uint8_t> green(red.size(), 0);
    for (std::size_t offset = 0; offset < red.size(); offset += 4) {
        red[offset] = 255;
        red[offset + 1] = 255;
        green[offset] = 255;
        green[offset + 2] = 255;
    }
    std::vector<std::uint8_t> output(static_cast<std::size_t>(outputSize * outputSize * 4), 0);
    mo::PixelBuffer redImage{red.data(), sourceSize, sourceSize, sourceSize * 4,
                             0, 0, mo::ComponentType::UInt8, 0, 1, 2, 3, 255.0F};
    mo::PixelBuffer greenImage{green.data(), sourceSize, sourceSize, sourceSize * 4,
                               0, 0, mo::ComponentType::UInt8, 0, 1, 2, 3, 255.0F};
    mo::PixelBuffer outputImage{output.data(), outputSize, outputSize, outputSize * 4,
                                0, 0, mo::ComponentType::UInt8, 0, 1, 2, 3, 255.0F};
    mo::RenderParams params{};
    params.formation = mo::Formation::Linear;
    params.cloneCount = 2;
    params.centerX = 32.0F;
    params.centerY = 32.0F;
    params.spacingX = 24.0F;
    params.cloneSize = 10.0F;
    params.sourceSelection = mo::SourceSelection::Cycle;
    mo::clear(outputImage);
    mo::renderSources(outputImage, std::vector<mo::PixelBuffer>{redImage, greenImage}, params);
    const std::size_t left = static_cast<std::size_t>((32 * outputSize + 20) * 4);
    const std::size_t right = static_cast<std::size_t>((32 * outputSize + 44) * 4);
    require(output[left + 1] > 240 && output[left + 2] < 10,
            "First cycled source was not used by the first clone.");
    require(output[right + 2] > 240 && output[right + 1] < 10,
            "Second cycled source was not used by the second clone.");
}

void testBezierPathUsesEndpointsAndAlignment() {
    mo::RenderParams params{};
    params.formation = mo::Formation::Path;
    params.cloneCount = 5;
    params.pathStartX = 10.0F;
    params.pathStartY = 20.0F;
    params.pathControl1X = 40.0F;
    params.pathControl1Y = 20.0F;
    params.pathControl2X = 70.0F;
    params.pathControl2Y = 20.0F;
    params.pathEndX = 100.0F;
    params.pathEndY = 20.0F;
    params.pathAlign = true;
    const auto clones = mo::makeInstances(params);
    require(near(clones.front().x, 10.0F) && near(clones.front().y, 20.0F),
            "Bezier path did not begin at its Start point.");
    require(near(clones.back().x, 100.0F) && near(clones.back().y, 20.0F),
            "Bezier path did not end at its End point.");
    for (const auto& clone : clones) {
        require(near(clone.rotationDegrees, 0.0F),
                "Bezier tangent alignment is incorrect on a horizontal path.");
    }
}

void testVariationIsDeterministicAndSeeded() {
    mo::RenderParams params{};
    params.formation = mo::Formation::Linear;
    params.cloneCount = 12;
    params.positionJitterX = 40.0F;
    params.positionJitterY = 30.0F;
    params.scaleJitter = 0.5F;
    params.rotationJitter = 90.0F;
    params.opacityJitter = 0.5F;
    params.variationSeed = 25;
    const auto first = mo::makeInstances(params);
    const auto second = mo::makeInstances(params);
    for (std::size_t index = 0; index < first.size(); ++index) {
        require(near(first[index].x, second[index].x) &&
                    near(first[index].y, second[index].y) &&
                    near(first[index].scale, second[index].scale) &&
                    near(first[index].rotationDegrees, second[index].rotationDegrees) &&
                    near(first[index].opacity, second[index].opacity),
                "Variation changed without changing its seed.");
    }
    params.variationSeed = 26;
    const auto changed = mo::makeInstances(params);
    require(!near(first.front().x, changed.front().x) || !near(first.front().y, changed.front().y),
            "Variation seed did not produce a different arrangement.");
}

void testPaletteCyclesAndRandomizesDeterministically() {
    mo::RenderParams params{};
    params.cloneCount = 8;
    params.paletteEnabled = true;
    params.paletteSize = 3;
    params.paletteSelection = mo::SourceSelection::Cycle;
    auto clones = mo::makeInstances(params);
    require(clones[0].colorIndex == 0 && clones[1].colorIndex == 1 &&
                clones[2].colorIndex == 2 && clones[3].colorIndex == 0,
            "Palette cycle did not follow clone order.");

    params.paletteSelection = mo::SourceSelection::Random;
    params.paletteSeed = 91;
    const auto first = mo::makeInstances(params);
    const auto second = mo::makeInstances(params);
    for (std::size_t index = 0; index < first.size(); ++index) {
        require(first[index].colorIndex == second[index].colorIndex,
                "Random palette changed without changing its seed.");
        require(first[index].colorIndex >= 0 && first[index].colorIndex < 3,
                "Random palette selected a color outside Color Count.");
    }
}

void testStepProgressAndReverseOrder() {
    mo::RenderParams params{};
    params.formation = mo::Formation::Linear;
    params.cloneCount = 3;
    params.centerX = 0.0F;
    params.spacingX = 10.0F;
    params.step.enabled = true;
    params.step.positionX = 100.0F;
    params.step.falloff = 0.01F;

    params.step.progress = 0.0F;
    auto clones = mo::makeInstances(params);
    require(near(clones[0].x, -10.0F) && near(clones[2].x, 10.0F),
            "Step changed clones at zero progress.");

    params.step.progress = 1.0F;
    clones = mo::makeInstances(params);
    require(near(clones[0].x, 90.0F) && near(clones[2].x, 110.0F),
            "Step did not reach every clone at full progress.");

    params.step.progress = 1.0F / 3.0F;
    params.step.reverse = true;
    clones = mo::makeInstances(params);
    require(near(clones[0].x, -10.0F) && near(clones[2].x, 110.0F),
            "Reverse Step did not begin with the last clone.");
}

void testFormationRotationRotatesModuleAndClones() {
    mo::RenderParams params{};
    params.formation = mo::Formation::Linear;
    params.cloneCount = 2;
    params.centerX = 100.0F;
    params.centerY = 100.0F;
    params.spacingX = 20.0F;
    params.formationRotation = 90.0F;
    const auto clones = mo::makeInstances(params);
    require(near(clones[0].x, 100.0F) && near(clones[0].y, 90.0F) &&
                near(clones[1].x, 100.0F) && near(clones[1].y, 110.0F),
            "Formation rotation did not rotate positions around Center.");
    require(near(clones[0].rotationDegrees, 90.0F),
            "Formation rotation did not rotate clone orientation.");
}

void testCircleSupportsConcentricRings() {
    mo::RenderParams params{};
    params.formation = mo::Formation::Circle;
    params.cloneCount = 30;
    params.circleRings = 4;
    params.centerX = 100.0F;
    params.centerY = 100.0F;
    params.radius = 200.0F;
    const auto clones = mo::makeInstances(params);
    std::array<int, 4> ringCounts{};
    for (const auto& clone : clones) {
        const float distance = std::hypot(clone.x - params.centerX, clone.y - params.centerY);
        int matchedRing = -1;
        for (int ring = 0; ring < 4; ++ring) {
            if (near(distance, 50.0F * static_cast<float>(ring + 1))) {
                matchedRing = ring;
                ++ringCounts[static_cast<std::size_t>(ring)];
                break;
            }
        }
        require(matchedRing >= 0, "Circle clone was not placed on a concentric ring.");
    }
    require(ringCounts[0] >= 1 && ringCounts[0] <= ringCounts[1] &&
                ringCounts[1] <= ringCounts[2] && ringCounts[2] <= ringCounts[3],
            "Concentric Circle did not allocate more clones to larger rings.");
}

void testZCircleHasDepthAndRandomSizeIsStable() {
    mo::RenderParams params{};
    params.formation = mo::Formation::ZCircle;
    params.cloneCount = 12;
    params.centerY = 100.0F;
    params.zCircleTilt = 0.0F;
    params.zCircleDepthScale = 0.8F;
    const auto depthOnly = mo::makeInstances(params);
    require(depthOnly.front().depth <= depthOnly.back().depth &&
                depthOnly.front().scale < depthOnly.back().scale,
            "Z Circle did not sort and scale by depth.");
    params.appearanceRandomSize = 0.4F;
    const auto first = mo::makeInstances(params);
    const auto second = mo::makeInstances(params);
    bool foundDifferentSize = false;
    for (std::size_t index = 0; index < first.size(); ++index) {
        require(near(first[index].y, 100.0F), "Zero-tilt Z Circle must project to a horizontal ring.");
        require(near(first[index].scale, second[index].scale), "Random Size is not deterministic.");
        foundDifferentSize = foundDifferentSize || !near(first[index].scale, first.front().scale);
    }
    require(foundDifferentSize, "Random Size did not vary clone scale.");

    params.appearanceRandomSize = 0.0F;
    params.zCircleTilt = 35.0F;
    params.zCircleRotation = 0.0F;
    params.zCircleSpeed = 90.0F;
    params.timeSeconds = 0.0F;
    const auto orbitStart = mo::makeInstances(params);
    params.timeSeconds = 0.5F;
    const auto orbitLater = mo::makeInstances(params);
    bool orbitMoved = false;
    for (std::size_t index = 0; index < orbitStart.size(); ++index) {
        orbitMoved = orbitMoved || !near(orbitStart[index].x, orbitLater[index].x) ||
                                   !near(orbitStart[index].depth, orbitLater[index].depth);
    }
    require(orbitMoved, "Z Circle Rotation/Speed did not move clones around the depth ring.");
}

void testTimedVariationAndWiggleAnimate() {
    mo::RenderParams params{};
    params.formation = mo::Formation::Linear;
    params.cloneCount = 1;
    params.columns = 1;
    params.positionJitterX = 50.0F;
    params.variationSpeed = 1.0F;
    params.timeSeconds = 0.0F;
    const auto variationStart = mo::makeInstances(params);
    params.timeSeconds = 0.25F;
    const auto variationLater = mo::makeInstances(params);
    require(!near(variationStart[0].x, variationLater[0].x),
            "Variation Speed did not animate Variation.");

    params.positionJitterX = 0.0F;
    params.variationSpeed = 0.0F;
    params.wiggle.enabled = true;
    params.wiggle.speed = 1.0F;
    params.wiggle.positionY = 40.0F;
    params.timeSeconds = 0.0F;
    const auto wiggleStart = mo::makeInstances(params);
    params.timeSeconds = 0.25F;
    const auto wiggleLater = mo::makeInstances(params);
    require(!near(wiggleStart[0].y, wiggleLater[0].y),
            "Individual Wiggle did not animate clone position.");

    params.wiggle.continuous = false;
    params.timeSeconds = 0.0F;
    const auto frozenStart = mo::makeInstances(params);
    params.timeSeconds = 2.0F;
    const auto frozenLater = mo::makeInstances(params);
    require(near(frozenStart[0].y, frozenLater[0].y),
            "Disabled Continuous Motion did not freeze Individual Wiggle.");
}

void testTwoEffectorsCombineFromOriginalPosition() {
    mo::RenderParams params{};
    params.formation = mo::Formation::Linear;
    params.cloneCount = 1;
    params.columns = 1;
    params.centerX = 200.0F;
    params.centerY = 100.0F;
    auto configure = [&](mo::EffectorParams& effector, const float positionX) {
        effector.enabled = true;
        effector.centerX = 200.0F;
        effector.centerY = 100.0F;
        effector.radius = 100.0F;
        effector.innerRadius = 0.0F;
        effector.positionX = positionX;
        effector.positionY = 0.0F;
        effector.scaleAmount = 0.0F;
        effector.rotationAmount = 0.0F;
        effector.targetOpacity = 1.0F;
    };
    configure(params.effector, 50.0F);
    configure(params.effector2, 100.0F);
    params.effector2.colorEnabled = true;
    params.effector2.colorAmount = 1.0F;
    const auto clones = mo::makeInstances(params);
    require(near(clones[0].x, 350.0F),
            "Two Effectors did not combine their transforms from the original clone position.");
    require(near(clones[0].effectorColorMix2, 1.0F),
            "Effector 2 color influence was not stored.");
}

void testConnectionsAndLineEffectorRender() {
    constexpr int width = 128;
    constexpr int height = 128;
    std::vector<std::uint8_t> pixels(static_cast<std::size_t>(width * height * 4), 0);
    mo::ImageView image{pixels.data(), width, height, width * 4};
    mo::RenderParams params{};
    params.formation = mo::Formation::Linear;
    params.cloneCount = 2;
    params.centerX = 64.0F;
    params.centerY = 64.0F;
    params.spacingX = 80.0F;
    params.cloneSize = 10.0F;
    params.connections.enabled = true;
    params.connections.mode = mo::ConnectionMode::Sequence;
    params.connections.maxDistance = 200.0F;
    params.connections.thickness = 4.0F;
    params.connections.opacity = 1.0F;
    mo::clear(image);
    mo::render(image, params);
    const std::size_t midpointAlpha = static_cast<std::size_t>((64 * width + 64) * 4 + 3);
    require(pixels[midpointAlpha] > 200, "Connections did not render between clones.");

    params.lineEffector.enabled = true;
    params.lineEffector.centerX = 64.0F;
    params.lineEffector.centerY = 64.0F;
    params.lineEffector.radius = 100.0F;
    params.lineEffector.innerRadius = 0.0F;
    params.lineEffector.falloff = mo::FalloffType::Constant;
    params.lineEffector.strength = 1.0F;
    params.lineEffector.targetOpacity = 0.0F;
    params.lineEffector.scaleAmount = 0.0F;
    mo::clear(image);
    mo::render(image, params);
    require(pixels[midpointAlpha] == 0, "Line Effector did not animate connection opacity.");
}

template <typename Component>
void testNativeBuffer(const mo::ComponentType type, const float maximum, const char* message) {
    constexpr int width = 64;
    constexpr int height = 64;
    std::vector<Component> pixels(static_cast<std::size_t>(width * height * 4), Component{});
    mo::PixelBuffer image{pixels.data(), width, height,
                          static_cast<std::ptrdiff_t>(width * 4 * sizeof(Component)),
                          0, 0, type, 0, 1, 2, 3, maximum};
    mo::RenderParams params{};
    params.cloneCount = 1;
    params.columns = 1;
    params.centerX = 32.0F;
    params.centerY = 32.0F;
    params.cloneSize = 24.0F;
    mo::clear(image);
    mo::render(image, params);
    const auto alpha = pixels[static_cast<std::size_t>((32 * width + 32) * 4)];
    require(static_cast<float>(alpha) > maximum * 0.9F, message);
}

}  // namespace

int main() {
    try {
        testGridIsCentered();
        testCircleClosesWithoutDuplicateEndpoint();
        testScatterIsDeterministic();
        testRendererWritesOneBuffer();
        testInternalCircleHasAntialiasedEdges();
        testEffectorUsesItsOwnCenterAndRadius();
        testGlobeProjectsDepthAndSortsBackToFront();
        testSourceLayerPixelsAreCloned();
        testMultiSourceCyclesByCloneIndex();
        testBezierPathUsesEndpointsAndAlignment();
        testVariationIsDeterministicAndSeeded();
        testPaletteCyclesAndRandomizesDeterministically();
        testStepProgressAndReverseOrder();
        testFormationRotationRotatesModuleAndClones();
        testCircleSupportsConcentricRings();
        testZCircleHasDepthAndRandomSizeIsStable();
        testTimedVariationAndWiggleAnimate();
        testTwoEffectorsCombineFromOriginalPosition();
        testConnectionsAndLineEffectorRender();
        testNativeBuffer<std::uint16_t>(mo::ComponentType::UInt16, 32768.0F,
                                        "16-bpc native target was not rendered.");
        testNativeBuffer<float>(mo::ComponentType::Float32, 1.0F,
                                "32-bpc float native target was not rendered.");
        std::cout << "MO Effector Native core tests passed.\n";
        return 0;
    } catch (const std::exception& error) {
        std::cerr << "Test failure: " << error.what() << '\n';
        return 1;
    }
}

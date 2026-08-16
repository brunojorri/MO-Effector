#pragma once

#include <cstddef>
#include <cstdint>
#include <array>
#include <vector>

namespace mo {

enum class Primitive : std::uint8_t {
    Circle = 0,
    Square = 1,
    Polygon = 2,
};

enum class Formation : std::uint8_t {
    Grid = 0,
    Linear = 1,
    Circle = 2,
    Scatter = 3,
    Globe = 4,
    Path = 5,
    ZCircle = 6,
};

enum class EffectorShape : std::uint8_t {
    Circle = 0,
    Box = 1,
    LinearX = 2,
    LinearY = 3,
};

enum class FalloffType : std::uint8_t {
    Linear = 0,
    Smooth = 1,
    EaseIn = 2,
    EaseOut = 3,
    Gaussian = 4,
    Constant = 5,
};

enum class SourceSelection : std::uint8_t {
    Cycle = 0,
    Random = 1,
};

enum class ConnectionMode : std::uint8_t {
    Sequence = 0,
    Nearest = 1,
    Distance = 2,
};

struct Color {
    float red{0.12F};
    float green{0.78F};
    float blue{1.0F};
    float alpha{1.0F};
};

struct EffectorParams {
    bool enabled{false};
    bool invert{false};
    EffectorShape shape{EffectorShape::Circle};
    FalloffType falloff{FalloffType::Smooth};
    float centerX{960.0F};
    float centerY{540.0F};
    float radius{300.0F};
    float innerRadius{0.0F};
    float power{2.0F};
    float strength{1.0F};
    float positionX{0.0F};
    float positionY{-100.0F};
    float scaleAmount{0.5F};
    float rotationAmount{45.0F};
    float targetOpacity{0.0F};
    bool colorEnabled{false};
    Color targetColor{1.0F, 0.25F, 0.1F, 1.0F};
    float colorAmount{1.0F};
};

struct StepParams {
    bool enabled{false};
    bool reverse{false};
    float progress{};
    float falloff{0.2F};
    float positionX{};
    float positionY{};
    float scaleAmount{};
    float rotationAmount{};
    float targetOpacity{1.0F};
};

struct WiggleParams {
    bool enabled{false};
    bool continuous{true};
    float speed{1.0F};
    float positionX{};
    float positionY{};
    float scaleAmount{};
    float rotationAmount{};
    float opacityAmount{};
};

struct ConnectionParams {
    bool enabled{false};
    ConnectionMode mode{ConnectionMode::Nearest};
    int connectionsPerClone{2};
    float maxDistance{300.0F};
    bool closeLoop{true};
    float thickness{2.0F};
    Color color{0.4F, 0.8F, 1.0F, 1.0F};
    float opacity{0.75F};
};

struct RenderParams {
    Primitive primitive{Primitive::Circle};
    Formation formation{Formation::Grid};
    int cloneCount{24};
    int rows{4};
    int columns{6};
    int polygonSides{6};
    std::uint32_t randomSeed{1};
    std::uint32_t variationSeed{1};
    float centerX{960.0F};
    float centerY{540.0F};
    float spacingX{160.0F};
    float spacingY{160.0F};
    float radius{320.0F};
    float formationRotation{};
    int circleRings{1};
    float zCircleTilt{25.0F};
    float zCircleRotation{};
    float zCircleSpeed{};
    float zCircleDepthScale{0.8F};
    float globeRotationX{};
    float globeRotationY{};
    float globeRotationZ{};
    float globeDepthScale{0.8F};
    float pathStartX{384.0F};
    float pathStartY{540.0F};
    float pathControl1X{672.0F};
    float pathControl1Y{216.0F};
    float pathControl2X{1248.0F};
    float pathControl2Y{864.0F};
    float pathEndX{1536.0F};
    float pathEndY{540.0F};
    bool pathAlign{true};
    float pathPositionOffset{};
    float pathRotationOffset{};
    float cloneSize{80.0F};
    float rotationDegrees{0.0F};
    float opacity{1.0F};
    float positionJitterX{};
    float positionJitterY{};
    float scaleJitter{};
    float rotationJitter{};
    float opacityJitter{};
    float variationSpeed{};
    float timeSeconds{};
    float appearanceRandomSize{};
    Color color{};
    bool paletteEnabled{false};
    bool paletteTintsSources{true};
    int paletteSize{4};
    std::array<Color, 4> palette{};
    SourceSelection paletteSelection{SourceSelection::Cycle};
    std::uint32_t paletteSeed{1};
    SourceSelection sourceSelection{SourceSelection::Cycle};
    std::uint32_t sourceRandomSeed{1};
    StepParams step{};
    WiggleParams wiggle{};
    ConnectionParams connections{};
    EffectorParams lineEffector{};
    EffectorParams effector{};
    EffectorParams effector2{};
};

struct CloneInstance {
    float x{};
    float y{};
    float scale{1.0F};
    float rotationDegrees{};
    float opacity{1.0F};
    float depth{};
    int index{};
    int colorIndex{};
    float effectorColorMix{};
    float effectorColorMix2{};
};

struct ImageView {
    std::uint8_t* pixels{};
    int width{};
    int height{};
    std::ptrdiff_t rowBytes{};
};

enum class ComponentType : std::uint8_t {
    UInt8,
    UInt16,
    Float32,
};

// Describes a packed four-channel host buffer without imposing a channel order.
// Channel indices are expressed in components, so the core can write directly
// into After Effects' native ARGB buffers and into RGBA test buffers alike.
struct PixelBuffer {
    void* pixels{};
    int width{};
    int height{};
    std::ptrdiff_t rowBytes{};
    int originX{};
    int originY{};
    ComponentType componentType{ComponentType::UInt8};
    int alphaIndex{3};
    int redIndex{0};
    int greenIndex{1};
    int blueIndex{2};
    float integerMaximum{255.0F};
};

[[nodiscard]] std::vector<CloneInstance> makeInstances(const RenderParams& params);
void clear(ImageView image) noexcept;
void render(ImageView image, const RenderParams& params);
void clear(PixelBuffer image) noexcept;
void render(PixelBuffer image, const RenderParams& params);
void renderSource(PixelBuffer image, PixelBuffer source, const RenderParams& params);
void renderSources(PixelBuffer image, const std::vector<PixelBuffer>& sources, const RenderParams& params);

}  // namespace mo

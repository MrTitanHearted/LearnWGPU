struct Vertex {
    @location(0) position: vec3<f32>,
    @location(1) normal: vec3<f32>,
    @location(2) uv: vec2<f32>,
}

struct VertexOut {
    @builtin(position) position: vec4<f32>,
    @location(0) uv: vec2<f32>,
}

struct UniformBuffer {
    model: mat4x4<f32>,
}

@group(0) @binding(0)
var<uniform> uniformBuffer: UniformBuffer;
@group(0) @binding(1)
var sampler2d: sampler;
@group(0) @binding(2)
var texture1: texture_2d<f32>;
@group(0) @binding(3)
var texture2: texture_2d<f32>;

@vertex
fn vs_main(in: Vertex) -> VertexOut {
    var out: VertexOut;
    out.position = uniformBuffer.model * vec4<f32>(in.position, 1.0);
    out.uv = in.uv;
    return out;
}

@fragment
fn fs_main(in: VertexOut) -> @location(0) vec4<f32> {
    return textureSample(texture1, sampler2d, in.uv) * textureSample(texture2, sampler2d, in.uv);
}

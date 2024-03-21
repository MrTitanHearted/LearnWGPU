struct VertexIn {
    @location(0) position: vec3<f32>,
    @location(1) normal: vec3<f32>,
    @location(2) uv: vec2<f32>,
}

struct VertexOut {
    @builtin(position) position: vec4<f32>,
    @location(0) normal: vec3<f32>,
    @location(1) uv: vec2<f32>,
}

struct Camera {
    projection: mat4x4<f32>,
    view: mat4x4<f32>,
}

@group(0) @binding(1)
var<uniform> camera: Camera;
@group(1) @binding(1)
var<uniform> model: mat4x4<f32>;

@vertex
fn vs_main(in: VertexIn) -> VertexOut {
    var out: VertexOut;
    out.position = camera.projection * camera.view * model * vec4<f32>(in.position, 1.0);
    out.normal = in.normal;
    out.uv = in.uv;
    return out;
}

struct FragmentIn {
    @location(0) normal: vec3<f32>,
    @location(1) uv: vec2<f32>,
}

@group(0) @binding(0)
var sampler2d: sampler;
@group(1) @binding(0)
var texture: texture_2d<f32>;

@fragment
fn fs_main(in: FragmentIn) -> @location(0) vec4<f32> {
    return textureSample(texture, sampler2d, in.uv);
}
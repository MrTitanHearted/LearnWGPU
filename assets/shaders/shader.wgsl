struct Vertex {
    @location(0) position: vec3<f32>,
    @location(1) color: vec3<f32>,
    @location(2) uv: vec2<f32>,
}

struct VertexOut {
    @builtin(position) position: vec4<f32>,
    @location(0) color: vec4<f32>,
    @location(1) uv: vec2<f32>,
}

struct Uniform {
    model: mat4x4<f32>,
}

@group(0) @binding(0)
var sampler2d: sampler;
@group(0) @binding(1)
var myTexture: texture_2d<f32>;
@group(0) @binding(2)
var<uniform> myUniform: Uniform;

@vertex
fn vs_main(in: Vertex) -> VertexOut {
    var out: VertexOut;
    out.position = myUniform.model * vec4<f32>(in.position, 1.0);
    out.color = vec4<f32>(in.color, 1.0);
    out.uv = in.uv;
    return out;
}

@fragment
fn fs_main(in: VertexOut) -> @location(0) vec4<f32> {
    return in.color * textureSample(myTexture, sampler2d, in.uv);
}

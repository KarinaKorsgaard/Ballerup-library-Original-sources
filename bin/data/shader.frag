#ifdef GL_ES
precision mediump float;
#endif

uniform sampler2D tex;
uniform sampler2D mask;
uniform vec2 res;

varying vec2 texCoordVarying;
void main()
{
    
    vec2 uv = gl_FragCoord.xy/res.xy;
    vec4 color = texture2D(tex,uv);
    vec4 mask = texture2D(mask,uv);
   // vec4 tex = texture2D(tex, res.xy/texCoordVarying);
   // float mask = texture2D(mask, texCoordVarying).r;
    gl_FragColor = vec4(vec3(color.r),mask.r);
}

#version 410 core
in vec2 TexCoords;
out vec4 FragColor;

uniform sampler2D lowResTexture;
uniform vec2 screenSize;

void main() {
    vec2 pixelSize = 1.0 / screenSize;
    vec2 coord = TexCoords * screenSize;
    vec2 pixelCoords = floor(coord + vec2(0.5)) * pixelSize;
    vec4 pixelColor = texture(lowResTexture, pixelCoords);
    if (pixelColor.a == 0.0) {
        discard;
    }
    FragColor = pixelColor;
}
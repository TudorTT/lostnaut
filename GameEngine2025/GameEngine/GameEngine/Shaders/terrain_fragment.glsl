#version 330 core

in vec3 FragPos;
in vec3 Normal;
in vec2 TexCoords;

out vec4 FragColor;

// lighting + view
uniform vec3 lightPos;
uniform vec3 lightColor;
uniform vec3 viewPos;

// samplers set by Mesh.draw (texture_diffuse1, texture_height1, ...)
uniform sampler2D texture_diffuse1;

void main()
{
    vec3 baseColor = texture(texture_diffuse1, TexCoords).rgb;

    // basic Blinn-Phong
    vec3 N = normalize(Normal);
    vec3 L = normalize(lightPos - FragPos);
    float diff = max(dot(N, L), 0.0);

    vec3 V = normalize(viewPos - FragPos);
    vec3 H = normalize(L + V);
    float spec = pow(max(dot(N, H), 0.0), 64.0);

    vec3 ambient = 0.12 * baseColor;
    vec3 diffuse = diff * baseColor * lightColor;
    vec3 specular = spec * vec3(1.0) * lightColor * 0.3;

    vec3 color = ambient + diffuse + specular;

    FragColor = vec4(color, 1.0);
}
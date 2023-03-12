#version 460 core
out vec4 FragColor;

in vec2 TexCoords;

uniform sampler2D gPosition;
uniform sampler2D gNormal;
uniform sampler2D gTexNoise;

uniform vec3 samples[64];

// ��������� (��, ��������, �������� ������������ �� � �������� uniform-����������, ����� ����� ����������� ����� ����������� ������)
int kernelSize = 64;
float radius = 0.5;
float bias = 0.025;

const vec2 noiseScale = vec2(800.0/4.0, 600.0/4.0); 

uniform mat4 projection;

void main() {
    // �������� ������� ������ ��� ��������� SSAO
    vec3 fragPos = texture(gPosition, TexCoords).xyz;
    vec3 normal = normalize(texture(gNormal, TexCoords).rgb);
    vec3 randomVec = normalize(texture(gTexNoise, TexCoords * noiseScale).xyz);

    // ������� TBN-������� ����� ������: �� ������������ ������������ � ������������ ����
    vec3 tangent = normalize(randomVec - normal * dot(randomVec, normal));
    vec3 bitangent = cross(normal, tangent);
    mat3 TBN = mat3(tangent, bitangent, normal);

    // ���������� �� ���� ������ ������� ���� � ������� ����������� ���������
    float occlusion = 0.0;
    for(int i = 0; i < kernelSize; ++i) {
        // �������� ������� ����� �������
        vec3 _sample = TBN * samples[i];
        _sample = fragPos + _sample * radius; 
        
        // ���������� ���������� ����� ������� (��� ��������� ������� �� ������/��������)
        vec4 offset = vec4(_sample, 1.0);
        offset = projection * offset; // �� ������������ ���� � ���������� ������������
        offset.xyz /= offset.w; // ������� �����������
        offset.xyz = offset.xyz * 0.5 + 0.5; // ���������� � ��������� 0.0 - 1.0
        
        // �������� �������� ������� ����� �������
        float sampleDepth = texture(gPosition, offset.xy).z; 
        
        // �������� ���������� � ������������
        float rangeCheck = smoothstep(0.0, 1.0, radius / abs(fragPos.z - sampleDepth));
        occlusion += (sampleDepth >= _sample.z + bias ? 1.0 : 0.0) * rangeCheck;
    }
    occlusion = 1.0 - (occlusion / kernelSize);
    
    FragColor = vec4(occlusion);
}


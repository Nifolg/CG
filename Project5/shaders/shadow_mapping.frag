
//ФРАГМЕНТНЫЙ ШЕЙДЕР - расчет для фрагментов объекта, а вершинный шейдер - расчет для вершин (вершин обычно гораздо меньше чем фпагментов)
//проверяем, находится ли фрагмент в тени в фрагментном шейдере
#version 330 core
out vec4 FragColor;

in VS_OUT {
    vec3 FragPos;
    vec3 Normal;
    vec2 TexCoords;
    vec4 FragPosLightSpace;
} fs_in;

uniform sampler2D diffuseTexture;
uniform sampler2D shadowMap;

uniform vec3 lightPos;
uniform vec3 viewPos;

float ShadowCalculation(vec4 fragPosLightSpace)
{
    //позиция фрагмента в пространстве источника света
    vec3 projCoords = fragPosLightSpace.xyz / fragPosLightSpace.w;//делим для нормализации
    
    // переводим в отрезок [0,1] , т.к. текстурные координаты лежат в интервале [0,1]
    projCoords = projCoords * 0.5 + 0.5;
    
    //По этим координатам смотрим значение глубины в текстуре — это будет глубина ближайщего к источнику света объекта
    //float closestDepth = texture(shadowMap, projCoords.xy).r;//смотрим глубину из карты глубин
    
    // глубина текущего фрагмента относительно источника света
    float currentDepth = projCoords.z;
    
    // calculate bias (based on depth map resolution and slope)
    vec3 normal = normalize(fs_in.Normal);
    vec3 lightDir = normalize(lightPos - fs_in.FragPos);
    
    //bias - это мы делаем небольшой сдвиг поверхности, чтобы не было теневой лесенки на поверхности пола
    
    float bias = max(0.05 * (1.0 - dot(normal, lightDir)), 0.005);
    // check whether current frag pos is in shadow
    // float shadow = currentDepth - bias > closestDepth  ? 1.0 : 0.0;
    
    // PCF
    float shadow = 0.0;//0.0 для освещенного объекта; 1.0 - в тени
    vec2 texelSize = 1.0 / textureSize(shadowMap, 0);
    
    //просто выбрать соседние тексели в карте глубины и усреднить результат
    for(int x = -1; x <= 1; ++x)
    {
        for(int y = -1; y <= 1; ++y)
        {
            float pcfDepth = texture(shadowMap, projCoords.xy + vec2(x, y) * texelSize).r;
            shadow += currentDepth - bias > pcfDepth  ? 1.0 : 0.0;
        }
    }
    shadow /= 9.0;
    
    
    // keep the shadow at 0.0 when outside the far_plane region of the light's frustum.
    //мы будем считать незатенёнными все объекты, для которых z координата больше единицы
    if(projCoords.z > 1.0)
        shadow = 0.0;
        
    return shadow;
}

void main()
{
    vec3 color = texture(diffuseTexture, fs_in.TexCoords).rgb;
    vec3 normal = normalize(fs_in.Normal);
    vec3 lightColor = vec3(0.3);
    // ambient
    vec3 ambient = 0.3 * color;
    // diffuse
    vec3 lightDir = normalize(lightPos - fs_in.FragPos);
    float diff = max(dot(lightDir, normal), 0.0);
    vec3 diffuse = diff * lightColor;
    // specular
    vec3 viewDir = normalize(viewPos - fs_in.FragPos);
    vec3 reflectDir = reflect(-lightDir, normal);
    float spec = 0.0;
    
    vec3 halfwayDir = normalize(lightDir + viewDir);
    spec = pow(max(dot(normal, halfwayDir), 0.0), 64.0);
    
    vec3 specular = spec * lightColor;
    
    // считаем тень
    float shadow = ShadowCalculation(fs_in.FragPosLightSpace);
    
    vec3 lighting = (ambient + (1.0 - shadow) * (diffuse + specular)) * color;
    //используем домножение на (1.0 - shadow)— т.е., в зависимости от того, насколько сильно фрагмент не затенён.
    
    FragColor = vec4(lighting, 1.0);
}




//ФРАГМЕНТНЫЙ ШЕЙДЕР - расчет для фрагментов объекта, а вершинный шейдер - расчет для вершин (вершин обычно гораздо меньше чем фпагментов)

#version 330 core
out vec4 FragColor;

in VS_OUT {
    vec3 FragPos;
    vec2 TexCoords;
    vec3 TangentLightPos;
    vec3 TangentViewPos;
    vec3 TangentFragPos;
} fs_in;

uniform sampler2D diffuseMap;
uniform sampler2D normalMap;
uniform sampler2D depthMap;

uniform float heightScale;
/*
vec2 ParallaxMapping(vec2 texCoords, vec3 viewDir)
{
    // number of depth layers
    const float minLayers = 8;
    const float maxLayers = 32;
    //направление взгляда определяет необходимое число итераций эффекта (в касательном пространстве положительная полуось Z направлена по нормали к поверхности). Если мы бы мы взглянули параллельно поверхности, то эффект бы использовал все 32 слоя.
    float numLayers = mix(maxLayers, minLayers, abs(dot(vec3(0.0, 0.0, 1.0), viewDir)));
    // calculate the size of each layer
    float layerDepth = 1.0 / numLayers;
    // depth of current layer
    float currentLayerDepth = 0.0;
    // the amount to shift the texture coordinates per layer (from vector P)
    vec2 P = viewDir.xy / viewDir.z * heightScale;
    vec2 deltaTexCoords = P / numLayers;
  
    // get initial values
    vec2  currentTexCoords     = texCoords;
    float currentDepthMapValue = texture(depthMap, currentTexCoords).r;
      
    while(currentLayerDepth < currentDepthMapValue)
    {
        // shift texture coordinates along direction of P
        currentTexCoords -= deltaTexCoords;
        // get depthmap value at current texture coordinates
        currentDepthMapValue = texture(depthMap, currentTexCoords).r;
        // get depth of next layer
        currentLayerDepth += layerDepth;
    }
    
    // get texture coordinates before collision (reverse operations)
    vec2 prevTexCoords = currentTexCoords + deltaTexCoords;

    // get depth after and before collision for linear interpolation
    float afterDepth  = currentDepthMapValue - currentLayerDepth;
    float beforeDepth = texture(depthMap, prevTexCoords).r - currentLayerDepth + layerDepth;
 
    // interpolation of texture coordinates
    float weight = afterDepth / (afterDepth - beforeDepth);
    vec2 finalTexCoords = prevTexCoords * weight + currentTexCoords * (1.0 - weight);

    return finalTexCoords;
}
*/

// входные параметры: inTexCoords  - исходные текстурные координаты,
// inViewDir  - вектор на наблюдателя в касательном пр-ве
// выходные параметры: lastDepthValue – глубина в найденной точке пересечения
// функция возвращает измененные текстурные координаты
vec2 reliefPM(vec2 inTexCoords, vec3 inViewDir)//, out float lastDepthValue) {
{
// ======
// код, повторяющий реализацию Steep PM
// ======
    //разбиение глубины на слои для итерационного прохода
    const float _minLayers = 2.0f;
    const float _maxLayers = 32.0f;
    float _numLayers = mix(_maxLayers, _minLayers, abs(dot(vec3(0., 0., 1.), inViewDir)));

    float deltaDepth = 1./_numLayers;
    // heightScale – юниформ для контроля выраженности PM
    //Вектор P
    vec2 deltaTexcoord = heightScale * inViewDir.xy/(inViewDir.z * _numLayers);
    // inViewDir.xy / inViewDir.z --Поскольку вектор viewDir нормализован, то его компонента z лежит в интервале [0, 1]. Когда вектор практически параллелен поверхности компонента z близка к нулю, а операция деления возвращает вектор  гораздо большей длины, чем в случае если viewDir близок к перпендикуляру к поверхности. Мы масштабируем вектор  так, чтобы он увеличивался при взгляде на поверхность под углом – это позволяет получить более реалистичный результат в таких случаях
    
    // get initial values
    vec2 currentTexCoords = inTexCoords;
    float currentLayerDepth = 0.;
    float currentDepthValue = texture(depthMap, currentTexCoords).r;
    
    //итерационный проход по слоям до тех пор, пока не будет найдена выборка из карты глубин, лежащая «выше» значения глубины текущего слоя
    
    while (currentDepthValue > currentLayerDepth) {
        // рассчитываем глубину следующего слоя
        currentLayerDepth += deltaDepth;
        
        // смещаем текстурные координаты вдоль вектора P
        currentTexCoords -= deltaTexcoord;
        
        // делаем выборку из карты глубин в текущих текстурных координатах
        currentDepthValue = texture(depthMap, currentTexCoords).r;
        //currentDepthValue = depthValue(currentTexCoords);
    }
// ======
// код реализации Relief PM
// ======

// уполовиниваем смещение текстурных координат и размер слоя глубины
    deltaTexcoord *= 0.5;
    deltaDepth *= 0.5;
// сместимся в обратном направлении от точки, найденной в Steep PM
    currentTexCoords += deltaTexcoord;
    currentLayerDepth -= deltaDepth;

// установим максимум итераций поиска…
    const int _reliefSteps = 5;
    int currentStep = _reliefSteps;
    while (currentStep > 0) {
        
        currentDepthValue = texture(depthMap, currentTexCoords).r;
        deltaTexcoord *= 0.5;
        deltaDepth *= 0.5;
// если выборка глубины больше текущей глубины слоя,
// то уходим в левую половину интервала
        if (currentDepthValue > currentLayerDepth) {
            currentTexCoords -= deltaTexcoord;
            currentLayerDepth += deltaDepth;
        }
// иначе уходим в правую половину интервала
        else {
            currentTexCoords += deltaTexcoord;
            currentLayerDepth -= deltaDepth;
        }
        currentStep--;
    }

    //lastDepthValue = currentDepthValue;
    return currentTexCoords;
}
void main()
{
    // V-A
    vec3 viewDir = normalize(fs_in.TangentViewPos - fs_in.TangentFragPos);
    
    vec2 texCoords = fs_in.TexCoords;
    
    // получить смещенные текстурные координаты с помощью Parallax Mapping
    //texCoords = ParallaxMapping(fs_in.TexCoords,  viewDir);
    //float k;
    texCoords = reliefPM(fs_in.TexCoords, viewDir);
    
    //текстурные координаты могут выпасть за пределы единичного интервала и, в зависимости от режима повторения текстуры (wrapping mode), вызвать появление нежелательных результатов
    //просто отбросим все фрагменты, для которых текстурные координаты оказались вне единичного интервала
    if(texCoords.x > 1.0 || texCoords.y > 1.0 || texCoords.x < 0.0 || texCoords.y < 0.0)
        discard;

    // делаем выборку из использующихся текстур
    // с использованием смещенных координат
    
    // выборка вектора из карты нормалей с областью значений [0,1]
    vec3 normal = texture(normalMap, texCoords).rgb;
    normal = normalize(normal * 2.0 - 1.0);//перевели векторы нормали в [0;1]
   
    // делаем выборку из использующихся текстур
    // с использованием смещенных координат
    //
    vec3 color = texture(diffuseMap, texCoords).rgb;
    // ambient
    vec3 ambient = 0.1 * color;
    // diffuse
    vec3 lightDir = normalize(fs_in.TangentLightPos - fs_in.TangentFragPos);
    float diff = max(dot(lightDir, normal), 0.0);
    vec3 diffuse = diff * color;
    // specular
    vec3 reflectDir = reflect(-lightDir, normal);
    vec3 halfwayDir = normalize(lightDir + viewDir);
    float spec = pow(max(dot(normal, halfwayDir), 0.0), 32.0);

    vec3 specular = vec3(0.2) * spec;
    FragColor = vec4(ambient + diffuse + specular, 1.0);
}


//ФРАГМЕНТНЫЙ ШЕЙДЕР - расчет для фрагментов объекта, а вершинный шейдер - расчет для вершин (вершин обычно гораздо меньше чем фпагментов)

#version 330 core
struct Material
{
    //vec3 ambient;// фоновое освещение обычно совпадает с диффузным
    sampler2D diffuse;//рассеивание света(чем ближе к источнику - тем ярче)
    sampler2D specular;//блик
    float shininess;//коэф-т силы блика
};
struct Light
{
    vec3 position; //не требуется для направленного источника(если другой источник, то нужно)
    
    vec3 direction;
    
    
    vec3 ambient;
    vec3 diffuse;
    vec3 specular;
    
    //коэффициенты для расчета затухания в зависимости от дистанции
    float constant;
    float linear;//линейный коэф.
    float quadratic;//квадратичный коэф.
};

struct DirLight//структура направленного источника света
{
    vec3 direction;
    
    vec3 ambient;// фоновое освещение обычно совпадает с диффузным
    vec3 diffuse;//рассеивание света(чем ближе к источнику - тем ярче)
    vec3 specular;//блик
};

struct PointLight//точечный источник отвещения(напр. "Солнце")
{
    vec3 position;
    
    //коэффициенты для расчета затухания в зависимости от дистанции
    float constant;
    float linear;//линейный коэф.
    float quadratic;//квадратичный коэф.

    vec3 ambient;// фоновое освещение обычно совпадает с диффузным
    vec3 diffuse;//рассеивание света(чем ближе к источнику - тем ярче)
    vec3 specular;//блик
};
  #define NUMBER_POINT_LIGHTS 4 //число точечных источников (например 4)
  uniform PointLight pointLights[NUMBER_POINT_LIGHTS];//массив структур PointLight


out vec4 FragColor;

in vec3 Normal;//принимаем вектор нормали
in vec3 FragPos;//принимаем позицию фрагмента вершины объекта
in vec2 TexCoords;//координаты текстуры

uniform DirLight dirLight;

uniform vec3 viewPos;//позиция наблюдателя(нужно для создания бликов)
uniform Material material;
uniform Light light;

//объявляем прототипы функций (возвращают vec3 с вычисленным цветом фрагмента)

//функция вычисления компонент от направленного источника света
vec3 CalcDirLight(DirLight light, vec3 normal, vec3 viewDir);

//функия вычисления компонент точечного источника
vec3 CalcPointLight(PointLight light, vec3 normal, vec3 fragPos, vec3 viewDir);




void main()
{
    /*
    //расчёт коэф-а затухания
    float distance    = length(light.position - FragPos);//вычислили расстояние от источника света до фрагмента
    
    float attenuation = 1.0 / (light.constant + light.linear * distance +
                                light.quadratic * (distance * distance));//это просто мы подставили значения в физическую формулу
    
    //ambient - фоновое освещение
    vec3 ambient  = light.ambient  * vec3(texture(material.diffuse, TexCoords));
    // (совпадает с диффузным освещением)
    
    //diffuse - рассеивание света(чем ближе к источнику - тем ярче)
    vec3 norm = normalize(Normal);
    vec3 lightDir=normalize(-light.direction);//вектор направления освещения от источника к фрагменту вершины
    /*
     направления обычно задается как ориентированный от источника. Поэтому проводится инверсия, а в переменной сохраняем вектор, направленный на источник света
     */
    /*
    float diff = max(dot(norm, lightDir), 0.0);//скалярным произведение получаем угол между вектором нормали и вектором освещенности
    
    //цвет от "диффузии"
    //делаем выборку из текстуры, чтобы извлечь значение диффузного цвета фрагмента
    vec3 diffuse = light.diffuse *diff * vec3(texture(material.diffuse,                                                                     TexCoords));
    
    //specular - блик(блеск)
    vec3 viewDir = normalize(viewPos - FragPos);//вектор направления взгляда
    vec3 reflectDir = reflect(-lightDir, norm);//вектор направления блика
    /*
     вектор lightDir в настоящее время указывает в обратную сторону, то есть от фрагмента к источнику света (направление зависит от порядка вычитания векторов, которое мы делали при вычислении вектора lightDir). Поэтому, для получения правильного вектора отражения, мы меняем его направление на противоположное посредством инверсии вектора lightDir
     */
    /*
    float spec = pow(max(dot(viewDir, reflectDir), 0.0), material.shininess);
    vec3 specular = light.specular * spec * vec3(texture(material.specular,
                                                                    TexCoords));
    /*
        Сначала вычисляется скалярное произведение векторов отражения и направления взгляда (с отсевом отрицательных значений), а затем результат возводится в 32-ю степень. Константное значение 32 задает силу блеска. Чем больше это значение, тем сильнее свет будет отражаться, а не рассеиваться, и тем меньше станет размер пятна блика
    */
    /*
    ambient  *= attenuation;
    diffuse  *= attenuation;
    specular *= attenuation;
    
    FragColor = vec4(ambient + diffuse + specular, 1.0);
    */
    
    // свойства
    vec3 norm = normalize(Normal);
    vec3 viewDir = normalize(viewPos - FragPos);

    // фаза 1: Направленный источник освещения
    vec3 result = CalcDirLight(dirLight, norm, viewDir);
    // фаза 2: Точечные источники
    for(int i = 0; i < NUMBER_POINT_LIGHTS; i++)
        result += CalcPointLight(pointLights[i], norm, FragPos, viewDir);
    // фаза 3: фонарик
    //result += CalcSpotLight(spotLight, norm, FragPos, viewDir);
    
    FragColor = vec4(result, 1.0);
}




vec3 CalcDirLight(DirLight light, vec3 normal, vec3 viewDir)
{
    vec3 lightDir = normalize(-light.direction);//вектор направления освещения от источника к фрагменту вершины
    /*
     направления обычно задается как ориентированный от источника. Поэтому проводится инверсия, а в переменной сохраняем вектор, направленный на источник света
     */
    
    
    // диффузное освещение
    float diff = max(dot(normal, lightDir), 0.0);//скалярным произведение получаем угол между вектором нормали и вектором освещенности
    
    // освещение зеркальных бликов
    vec3 reflectDir = reflect(-lightDir, normal);//вектор направления блика
    /*
     вектор lightDir в настоящее время указывает в обратную сторону, то есть от фрагмента к источнику света (направление зависит от порядка вычитания векторов, которое мы делали при вычислении вектора lightDir). Поэтому, для получения правильного вектора отражения, мы меняем его направление на противоположное посредством инверсии вектора lightDir
     */
    
    
    float spec = pow(max(dot(viewDir, reflectDir), 0.0), material.shininess);
    /*
           Сначала вычисляется скалярное произведение векторов отражения и направления взгляда (с отсевом отрицательных значений), а затем результат возводится в 32-ю степень. Константное значение 32 задает силу блеска. Чем больше это значение, тем сильнее свет будет отражаться, а не рассеиваться, и тем меньше станет размер пятна блика
       */
    
    // комбинируем результаты
    vec3 ambient  = light.ambient  * vec3(texture(material.diffuse, TexCoords));
    vec3 diffuse  = light.diffuse  * diff * vec3(texture(material.diffuse, TexCoords));
    vec3 specular = light.specular * spec * vec3(texture(material.specular, TexCoords));
    specular*=5.0f;

    return (ambient + diffuse + specular);
}

vec3 CalcPointLight(PointLight light, vec3 normal, vec3 fragPos, vec3 viewDir)
{
    vec3 lightDir = normalize(light.position - fragPos);//вектор направления освещения от источника к фрагменту вершины
    /*
     направления обычно задается как ориентированный от источника. Поэтому проводится инверсия, а в переменной сохраняем вектор, направленный на источник света
     */
    
    
    // диффузное освещение
    float diff = max(dot(normal, lightDir), 0.0);//скалярным произведение получаем угол между вектором нормали и вектором освещенности
    
    // освещение зеркальных бликов
    vec3 reflectDir = reflect(-lightDir, normal);//вектор направления блика
    /*
     вектор lightDir в настоящее время указывает в обратную сторону, то есть от фрагмента к источнику света (направление зависит от порядка вычитания векторов, которое мы делали при вычислении вектора lightDir). Поэтому, для получения правильного вектора отражения, мы меняем его направление на противоположное посредством инверсии вектора lightDir
     */
    
    
    float spec = pow(max(dot(viewDir, reflectDir), 0.0), material.shininess);
    /*
           Сначала вычисляется скалярное произведение векторов отражения и направления взгляда (с отсевом отрицательных значений), а затем результат возводится в 32-ю степень. Константное значение 32 задает силу блеска. Чем больше это значение, тем сильнее свет будет отражаться, а не рассеиваться, и тем меньше станет размер пятна блика
       */
    
    // затухание
    float distance = length(light.position - fragPos);
    float attenuation = 1.0 / (light.constant + light.linear * distance +
                   light.quadratic * (distance * distance));
    // комбинируем результаты
    vec3 ambient  = light.ambient  * vec3(texture(material.diffuse, TexCoords));
    vec3 diffuse  = light.diffuse  * diff * vec3(texture(material.diffuse, TexCoords));
    vec3 specular = light.specular * spec * vec3(texture(material.specular, TexCoords));
    ambient  *= attenuation;
    diffuse  *= attenuation;
    specular *= attenuation;
    specular*=5.0f;
    return (ambient + diffuse + specular);
}

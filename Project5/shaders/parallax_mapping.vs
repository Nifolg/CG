#version 330 core
layout (location = 0) in vec3 aPos;
layout (location = 1) in vec3 aNormal;
layout (location = 2) in vec2 aTexCoords;
layout (location = 3) in vec3 aTangent;
layout (location = 4) in vec3 aBitangent;

out VS_OUT {
    vec3 FragPos;
    vec2 TexCoords;
    vec3 TangentLightPos;
    vec3 TangentViewPos;
    vec3 TangentFragPos;
} vs_out;

uniform mat4 projection;
uniform mat4 view;
uniform mat4 model;

uniform vec3 lightPos;
uniform vec3 viewPos;

void main()
{
    vs_out.FragPos = vec3(model * vec4(aPos, 1.0));
    vs_out.TexCoords = aTexCoords;
    
    //преобразуем все вектора базиса касательного пространства в систему координат, в которой нам удобно работать – в данном случае это мировая система координат и мы умножаем вектора на модельную матрицу model
    vec3 T = normalize(mat3(model) * aTangent);
    vec3 B = normalize(mat3(model) * aBitangent);
    vec3 N = normalize(mat3(model) * aNormal);
    
    mat3 TBN = transpose(mat3(T, B, N));//получаем обратную матрицу(т.к. она ортогональна, то можно транспонировать, а не обращать)

    //перевод нужных векторов в касательную систему координат
    vs_out.TangentLightPos = TBN * lightPos;//вектор источника света
    vs_out.TangentViewPos  = TBN * viewPos;//вектор наблюдателя
    vs_out.TangentFragPos  = TBN * vs_out.FragPos;//вектор положения вершины
    
    gl_Position = projection * view * model * vec4(aPos, 1.0);
}

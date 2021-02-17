#version 330 core
layout (location = 0) in vec3 position;
layout (location = 1) in vec3 normal;
layout (location = 2) in vec2 texCoords;

out vec3 Normal;
out vec3 FragPos;
out vec2 TexCoords;

//матрицы преобразования
uniform mat4 model;
uniform mat4 view;
uniform mat4 projection;

void main()
{
    // !!! ЧИТАЕМ СПРАВА-НАЛЕВО
    gl_Position = projection * view * model * vec4(position, 1.0f);
    
    //преобразование позиции вершины в мировые координаты
    FragPos = vec3(model * vec4(position, 1.0f));//позиция текущего фрагмента
    
    Normal = mat3(transpose(inverse(model))) * normal;//матрица нормали для перемещения векторов нормалей из мировой системы коорднат в с.к. "вида" это нужно, чтобы сохранять вектора нормалей при неравномерном масштабировании
    TexCoords=texCoords;
}
//Местоположение текстуры чаще называется !!!текстурным блоком!!!

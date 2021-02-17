#version 330 core
layout (location = 0) in vec3 position;

//матрицы преобразования
uniform mat4 model;
uniform mat4 view;
uniform mat4 projection;

void main()
{
    // !!! ЧИТАЕМ СПРАВА-НАЛЕВО
    gl_Position = projection * view * model * vec4(position, 1.0f);
    
}
//Местоположение текстуры чаще называется !!!текстурным блоком!!!

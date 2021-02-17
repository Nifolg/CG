//
//  MATRIX_INIT.h
//  OpenGL
//
//  Created by Денис on 21/11/2019.
//  Copyright © 2019 Денис. All rights reserved.
//

#ifndef MATRIX_INIT_h
#define MATRIX_INIT_h

#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>
#include <glm/gtc/type_ptr.hpp>

void matrix_init4(glm::mat4 &mtrx)
{
    float mas[16]=
    {
        1.0f,   0.0f,  0.0f,   0.0f,
        0.0f,   1.0f,  0.0f,   0.0f,
        0.0f,   0.0f,  1.0f,   0.0f,
        0.0f,   0.0f,  0.0f,   1.0f
    };
    mtrx=glm::make_mat4(mas);
}

#endif /* MATRIX_INIT_h */

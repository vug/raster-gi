mat4 lookAt(vec3 eye, vec3 center, vec3 up) {
    vec3 f = normalize(center - eye);
    vec3 s = normalize(cross(f, up));
    vec3 u = cross(s, f);
    mat4 result = mat4(1.0);
    result[0][0] = s.x;
    result[1][0] = s.y;
    result[2][0] = s.z;
    result[0][1] = u.x;
    result[1][1] = u.y;
    result[2][1] = u.z;
    result[0][2] = -f.x;
    result[1][2] = -f.y;
    result[2][2] = -f.z;
    result[3][0] = -dot(s, eye);
    result[3][1] = -dot(u, eye);
    result[3][2] = dot(f, eye);
    return result;
}

mat4 perspective(float fov, float aspect, float near, float far) {
    float tanHalfFov = tan(fov / 2.0f);
    mat4 result = mat4(0);
    result[0][0] = 1.0f / (aspect * tanHalfFov);
    result[1][1] = 1.0f / tanHalfFov;
    result[2][2] = -(far + near) / (far - near);
    result[2][3] = -1.0f;
    result[3][2] = -(2.0f * far * near) / (far - near);
    return result;
}

void vert() {
    // vec3 eye = vec3(2 * cos(uTime), 1, 2 * sin(uTime)); 
    vec3 eye = vec3(5, -5, 5);
    // int sbIx = int(uTime) % 3;
    // vec3 eye = camPositions[sbIx];
    vec3 center = vec3(0, 0, 0);
    vec3 up = vec3(0, 1, 0);    

    const mat4 model = mat4(1);
    const mat4 view = lookAt(eye, center, up);
    const mat4 projection = perspective(3.1415 / 2, 1, 0.1, 100.0);
    const mat4 mvp = projection * view * model;
    gl_Position = mvp * vec4(aPosition, 1.0);    
}

const vec3 lightPos = vec3(1, 5, 1);
void frag() {
    const vec3 normal = normalize(vNormal);
    const vec3 lightDir = normalize(lightPos - vWorldPos);
    float diffuse = max(0, dot(normal, lightDir));
    //fragColor = vec4(vec3(diffuse), 1.0);
    fragColor = vec4(vColor + diffuse, 1.0);        
}
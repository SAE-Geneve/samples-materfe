#version 300 es
precision highp float;

const int number_of_spikes = 2;

out vec3 fragColor;

vec2 positions_for_full_star[3 * number_of_spikes];

vec3 colors[3 * number_of_spikes];

void setposition()
{
    int size_of_array = positions_for_full_star.length();
    int number_of_iterations = 0;
    for (int i = 0; i < size_of_array; i++)
    {
        switch (i % 3)
        {
            case 0:
                colors[i] = vec3(1.0, 1.0, 0.0);
                positions_for_full_star[i] = vec2(i, i);
                number_of_iterations++;
                break;
            case 1:
                colors[i] = vec3(1.0, 0.0, 1.0);
                positions_for_full_star[i] = vec2(i + (number_of_iterations % 100) / 100,
                i + (number_of_iterations % 100) / 100);
                number_of_iterations++;
                break;
            case 2:
                colors[i] = vec3(1.0, 0.5, 0.3);
                positions_for_full_star[i] = vec2(i + (number_of_iterations % 100) / 100,
                i + (number_of_iterations % 100) / 100);
                number_of_iterations++;
                break;
            default:
                break;
        }
    }
}


void main() {
    setposition();
    gl_Position = vec4(positions_for_full_star[gl_VertexID], 0.0, 1.0);
    fragColor = colors[gl_VertexID];
}

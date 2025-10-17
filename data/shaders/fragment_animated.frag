#version 460 core
out vec4 FragColor;

// Uniforms provided by your application
uniform float u_time;     // Time in seconds, for animation

void main()
{
    vec2 u_resolution = vec2(1200.0, 720.0);
    // 1. SET UP COORDINATES
    // We normalize the coordinates so that (0,0) is the center of the screen
    // and the y-axis ranges from -1.0 to 1.0, correcting for aspect ratio.
    vec2 uv = (2.0 * gl_FragCoord.xy - u_resolution.xy) / u_resolution.y;

    // 2. CREATE THE PATTERN 🌀
    // We'll combine two different motions: a twist and a radial ripple.

    // Layer 1: A twisting motion based on the angle from the center.
    // 'atan' gives us the angle, creating radial arms. We have 5 arms.
    // We animate the 'twist' value with time to make it rotate.
    float angle = atan(uv.y, uv.x);
    float twist = 0.5 * cos(u_time * 0.5);
    float pattern1 = sin(angle * 15.0 + twist);

    // Layer 2: An outward ripple based on the distance from the center.
    // 'length' gives us the distance, creating concentric circles.
    // We subtract time to make the waves move outwards.
    float radius = length(uv);
    float pattern2 = sin(radius * 15.0 - u_time * 2.0);

    // 3. COMBINE THE PATTERNS ✨
    // Adding the two patterns together creates a more complex effect.
    float final_pattern = pattern1 + pattern2;

    // 4. COLORIZE THE PATTERN 🎨
    // We use the cosine function with different phase shifts for each color channel.
    // This maps the black-and-white pattern to a vibrant, smoothly shifting gradient.
    vec3 color = 0.5 + 0.5 * cos(u_time * 0.8 + final_pattern * 3.14159 + vec3(0.0, 0.4, 0.8));

    // 5. SET THE FINAL FRAGMENT COLOR
    FragColor = vec4(color, 1.0);
}
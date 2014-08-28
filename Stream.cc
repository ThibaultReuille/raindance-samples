#include <raindance/Raindance.hh>
#include <raindance/Core/Camera/Camera.hh>
#include <raindance/Core/Primitives/Quad.hh>
#include <raindance/Core/Transformation.hh>

const std::string g_VertexShader =                          
"attribute vec3 a_Position;                                             \n"
"attribute vec2 a_Texcoord;                                             \n"
"                                                                       \n"
"uniform mat4 u_ModelViewProjectionMatrix;                              \n"
"uniform vec4 u_Color;"
"                                                                       \n"
"varying vec4 v_Color;                                                  \n"
"                                                                       \n"
"void main(void)                                                        \n"
"{                                                                      \n"
"    v_Color = u_Color;                                                 \n"
"    gl_Position = u_ModelViewProjectionMatrix * vec4(a_Position, 1.0); \n"
"}                                                                      \n";

const std::string g_FragmentShader =
"#ifdef GL_ES                \n"
"precision mediump float;    \n"
"#endif                      \n"
"                            \n"
"varying vec4 v_Color;       \n"
"                            \n"
"void main(void)             \n"
"{                           \n"
"    gl_FragColor = v_Color; \n"
"}                           \n";

class TimeSerie
{
public:

    class Tick
    {
    public:
        inline void data(const glm::vec3& data) { m_Data = data; }
        inline glm::vec3& data() { return m_Data; }

    private:
        glm::vec3 m_Data;
    };

    TimeSerie(size_t size, const glm::vec4 color)
    {
        m_Ticks.resize(size);

        m_Color = color;

        m_Quad.getVertexBuffer().mute("a_Normal", true);
        m_Quad.getVertexBuffer().mute("a_Texcoord", true);

        m_Begin = 0;
        m_End = 0;

        m_Shader = ResourceManager::getInstance().loadShader("shaders/time_serie", g_VertexShader, g_FragmentShader);
        m_Shader->dump();
    }

    virtual ~TimeSerie()
    {
        ResourceManager::getInstance().unload(m_Shader);
    }

    virtual void draw(Context& context, Camera& camera)
    {
        Transformation transformation;

        m_Shader->use();
        m_Shader->uniform("u_Color").set(m_Color);

        float width_div_size = 800 / static_cast<float>(m_Ticks.size()) ;

        size_t index = m_Begin;
        float x = 0.0;
        while (index != m_End)
        {
            transformation.set(glm::mat4());

            transformation.translate(glm::vec3(x, m_Ticks[index].data().y * width_div_size, 1.0));
            transformation.scale(glm::vec3(5.0, 5.0, 1.0));

            m_Shader->uniform("u_ModelViewProjectionMatrix").set(camera.getViewProjectionMatrix() * transformation.state());
            context.geometry().bind(m_Quad.getVertexBuffer(), *m_Shader);
            context.geometry().drawElements(GL_TRIANGLES, m_Quad.getTriangleBuffer().size() / sizeof(unsigned char), GL_UNSIGNED_BYTE, m_Quad.getTriangleBuffer().ptr());
            context.geometry().unbind(m_Quad.getVertexBuffer());

            index = (index + 1) % m_Ticks.size();
            x += width_div_size;
        }
    }

    virtual void push(Tick& tick)
    {
        m_Ticks[m_End] = tick;

        LOG("%f, %f\n", tick.data().x, tick.data().y);
        m_End = (m_End + 1) % m_Ticks.size();

        if (m_End == m_Begin)
            m_Begin = (m_Begin + 1) % m_Ticks.size();
    }

protected:
    std::vector<Tick> m_Ticks;
    Quad m_Quad;
    size_t m_Begin;
    size_t m_End;
    Shader::Program* m_Shader;
    glm::vec4 m_Color;
};

class Demo : public RainDance
{
public:
    Demo()
    : m_TimeSerie1(NULL), m_TimeSerie2(NULL)
    {
    }

    virtual ~Demo()
    {
        SAFE_DELETE(m_TimeSerie1);
        SAFE_DELETE(m_TimeSerie2);
    }

    virtual void initialize()
    {
        float width = static_cast<float>(glutGet(GLUT_WINDOW_WIDTH));
        float height = static_cast<float>(glutGet(GLUT_WINDOW_HEIGHT));

        m_Camera.setOrthographicProjection(-10, width + 10, - height - 10, height + 10, -10.0, 10.0);
        m_Camera.lookAt(glm::vec3(0, 0, 1.0), glm::vec3(0, 0, 0), glm::vec3(0, 1, 0));

        m_TimeSerie1 = new TimeSerie(200, glm::vec4(SKY_BLUE, 1.0));
        m_TimeSerie2 = new TimeSerie(50, glm::vec4(DRAGON_GREEN, 1.0));

        glClearColor(0.2, 0.2, 0.2, 1.0);
        glEnable(GL_DEPTH_TEST);
    }

    virtual void destroy()
    {
    }

    virtual void draw()
    {
        glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

        m_TimeSerie1->draw(context(), m_Camera);
        m_TimeSerie2->draw(context(), m_Camera);

        finish();
    }

    virtual void idle()
    {
        static bool _first = true;
        static float _lastTime1;
        static float _lastTime2;
        static float _value1 = 0.0;
        static float _value2 = 0.0;

        TimeSerie::Tick tick;

        if (_first)
        {
            context().clock().start();
            _lastTime1 = _lastTime2 = context().clock().seconds();
            _first = false;
        }

        float t = context().clock().seconds();

        if (t - _lastTime1 >= 0.01f)
        {
            _value1 += 2.0 * RANDOM_FLOAT(-1.0, 1.0);
            tick.data(glm::vec3(t, _value1, 0.0));
            m_TimeSerie1->push(tick);
            _lastTime1 = t;
        }

        if (t - _lastTime2 >= 0.2f)
        {
            _value2 += 2.0 * RANDOM_FLOAT(-1.0, 1.0);
            tick.data(glm::vec3(t, _value2, 0.0));
            m_TimeSerie2->push(tick);
            _lastTime2 = t;
        }

        glutPostRedisplay();
    }

private:
    Camera m_Camera;
    TimeSerie* m_TimeSerie1;
    TimeSerie* m_TimeSerie2;
};

int main(int argc, char** argv)
{
    Demo demo;

    demo.create(argc, argv);

    demo.addWindow("Stream", 800, 600);

    demo.initialize();
    demo.run();

    demo.destroy();
}

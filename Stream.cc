#include <raindance/Raindance.hh>
#include <raindance/Core/Camera/Camera.hh>
#include <raindance/Core/Primitives/Quad.hh>
#include <raindance/Core/Primitives/WideLine.hh>
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
        Tick() {}
        Tick(const glm::vec3& data) { m_Data = data; }
        inline void data(const glm::vec3& data) { m_Data = data; }
        inline glm::vec3& data() { return m_Data; }

    private:
        glm::vec3 m_Data;
    };

    TimeSerie(size_t size, const glm::vec4 color)
    {
        m_Ticks.resize(size);

        m_Color = color;

        m_Quad = new Quad(glm::vec3(-0.5, -0.5, 0.0));
        m_Quad->getVertexBuffer().mute("a_Normal", true);
        m_Quad->getVertexBuffer().mute("a_Texcoord", true);

        m_WideLine.setColor(0, color);
        m_WideLine.setColor(1, color);
        m_WideLine.update();

        m_Begin = 0;
        m_End = 0;

        m_Shader = ResourceManager::getInstance().loadShader("shaders/time_serie",
            g_VertexShader, g_FragmentShader);
        // m_Shader->dump();

        m_WideLineShader = ResourceManager::getInstance().loadShader("shaders/wideline",
            Resources_Shaders_Primitives_wideline_vert, sizeof(Resources_Shaders_Primitives_wideline_vert),
            Resources_Shaders_Primitives_wideline_frag, sizeof(Resources_Shaders_Primitives_wideline_frag));
        m_WideLineShader->dump();
    }

    virtual ~TimeSerie()
    {
        ResourceManager::getInstance().unload(m_Shader);
        ResourceManager::getInstance().unload(m_WideLineShader);
    }

    virtual void draw(Context& context, Camera& camera)
    {
        drawLines(context, camera);
        //drawQuads(context, camera);
    }

    virtual void drawLines(Context& context, Camera& camera)
    {
        m_WideLineShader->use();

        m_WideLineShader->uniform("u_Mode").set(0.0f);
        m_WideLineShader->uniform("u_ModelViewProjection").set(camera.getViewProjectionMatrix());
        // m_Shader->uniform("u_ExtrudeDirection").set(glm::vec3(0.0, 1.0, 0.0));
        // m_Shader->uniform("u_Texture").set(Icon->getTexture(m_TextureID));

        float height_div_size = 600 / static_cast<float>(abs(100 - (-100)));
        float width_div_size = 800 / static_cast<float>(m_Ticks.size()) ;

        float x = 0.0;
        size_t index = m_Begin;

        glm::vec3 lastPos;
        glm::vec3 currPos = glm::vec3(0.0, m_Ticks[index].data().y * height_div_size, 1.0);
        glm::vec4 color = glm::vec4(1.2, 1.2, 1.2, 0.8);
        
        do
        {
            index = (index + 1) % m_Ticks.size();
            if (index == m_End)
                break;

            x += width_div_size;
            lastPos = currPos;
            currPos = glm::vec3(x, m_Ticks[index].data().y * height_div_size, 1.0);

            m_WideLineShader->use();
            m_WideLineShader->uniform("u_StartPosition").set(lastPos);
            m_WideLineShader->uniform("u_EndPosition").set(currPos);
            m_WideLineShader->uniform("u_Tint").set(color);

            context.geometry().bind(m_WideLine.getVertexBuffer(), *m_WideLineShader);
            context.geometry().drawArrays(GL_LINES, 0, 2 * sizeof(WideLine::Vertex));
            context.geometry().unbind(m_WideLine.getVertexBuffer());

        } while (true);
    }

    virtual void drawQuads(Context& context, Camera& camera)
    {
        Transformation transformation;

        m_Shader->use();
        m_Shader->uniform("u_Color").set(m_Color);

        float height_div_size = 600 / static_cast<float>(abs(100 - (-100)));
        float width_div_size = 800 / static_cast<float>(m_Ticks.size()) ;

        size_t index = m_Begin;
        float x = 0.0;
        while (index != m_End)
        {
            transformation.set(glm::mat4());

            transformation.translate(glm::vec3(x, m_Ticks[index].data().y * height_div_size, 1.0));
            transformation.scale(glm::vec3(5.0, 5.0, 1.0));

            m_Shader->uniform("u_ModelViewProjectionMatrix").set(camera.getViewProjectionMatrix() * transformation.state());
            context.geometry().bind(m_Quad->getVertexBuffer(), *m_Shader);
            context.geometry().drawElements(GL_TRIANGLES, m_Quad->getTriangleBuffer().size() / sizeof(unsigned char), GL_UNSIGNED_BYTE, m_Quad->getTriangleBuffer().ptr());
            context.geometry().unbind(m_Quad->getVertexBuffer());

            index = (index + 1) % m_Ticks.size();
            x += width_div_size;
        }
    }

    virtual void push(Tick tick)
    {
        m_Ticks[m_End] = tick;

        m_End = (m_End + 1) % m_Ticks.size();

        if (m_End == m_Begin)
            m_Begin = (m_Begin + 1) % m_Ticks.size();
    }

    virtual void getStats(size_t n, float* average, float* min, float* max)
    {
        if (n > m_Ticks.size())
        {
            LOG("Can't calculate stats, n is out of bounds!\n");
            return;
        }

        *min = std::numeric_limits<float>::max();
        *max = -std::numeric_limits<float>::max();

        float value;
        float sum = 0.0;
        float count = 0.0;
        size_t index = m_End;
        while (n > 0)
        {
            if (index == 0)
                index = m_Ticks.size();
            index--;

            value = m_Ticks[index].data().y;

            if (value > *max)
                *max = value;
            if (value < *min)
                *min = value;

            sum += value;
            count += 1.0;

            if (index == m_Begin)
                break;
            n--;
        }

        *average = sum / count;
    }

protected:
    std::vector<Tick> m_Ticks;
    size_t m_Begin;
    size_t m_End;

    Quad* m_Quad;
    WideLine m_WideLine;
    Shader::Program* m_WideLineShader;
    // Icon* m_WideLineIcon;

    Shader::Program* m_Shader;
    glm::vec4 m_Color;
};

class Demo : public RainDance
{
public:
    Demo()
    {
    }

    virtual ~Demo()
    {
        SAFE_DELETE(m_TimeSerie);
        SAFE_DELETE(m_TimeSerieAvg);
        SAFE_DELETE(m_TimeSerieMin);
        SAFE_DELETE(m_TimeSerieMax);
    }

    virtual void initialize()
    {
        float width = static_cast<float>(glutGet(GLUT_WINDOW_WIDTH));
        float height = static_cast<float>(glutGet(GLUT_WINDOW_HEIGHT));

        m_Camera.setOrthographicProjection(-10, width + 10, - height - 10, height + 10, -10.0, 10.0);
        m_Camera.lookAt(glm::vec3(0, 0, 1.0), glm::vec3(0, 0, 0), glm::vec3(0, 1, 0));

        m_TimeSerie = new TimeSerie(500, glm::vec4(WHITE, 1.0));
        m_TimeSerieAvg = new TimeSerie(500, glm::vec4(LOVE_RED, 1.0));
        m_TimeSerieMin = new TimeSerie(500, glm::vec4(AQUAMARINE, 1.0));
        m_TimeSerieMax = new TimeSerie(500, glm::vec4(AQUAMARINE, 1.0));

        glClearColor(0.1, 0.1, 0.1, 1.0);
        glEnable(GL_DEPTH_TEST);
    }

    virtual void destroy()
    {
    }

    virtual void draw()
    {
        glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

        m_TimeSerie->draw(context(), m_Camera);
        m_TimeSerieAvg->draw(context(), m_Camera);
        m_TimeSerieMin->draw(context(), m_Camera);
        m_TimeSerieMax->draw(context(), m_Camera);

        finish();
    }

    virtual void idle()
    {
        static bool _first = true;
        static float _lastTime;
        static float _value = 0.0;

        if (_first)
        {
            context().clock().start();
            _lastTime = context().clock().seconds();
            _first = false;
        }

        float t = context().clock().seconds();

        float min = -100.0f;
        float max = +100.0f;

        float moving_min;
        float moving_max;
        float moving_avg;

        if (t - _lastTime >= 0.001f)
        {
            _value += RANDOM_FLOAT(-5.0, 5.0);
            _value = std::max(min, std::min(max, _value));
            LOG("%f\t%f\n", t, _value);

            m_TimeSerie->push(TimeSerie::Tick(glm::vec3(t, _value, 0.0)));

            m_TimeSerie->getStats(50, &moving_avg, &moving_min, &moving_max);
            m_TimeSerieAvg->push(TimeSerie::Tick(glm::vec3(t, moving_avg, 0.0)));
            m_TimeSerieMin->push(TimeSerie::Tick(glm::vec3(t, moving_min, 0.0)));
            m_TimeSerieMax->push(TimeSerie::Tick(glm::vec3(t, moving_max, 0.0)));

            _lastTime = t;     
        }

        glutPostRedisplay();
    }

private:
    Camera m_Camera;
    TimeSerie* m_TimeSerie;
    TimeSerie* m_TimeSerieAvg;
    TimeSerie* m_TimeSerieMin;
    TimeSerie* m_TimeSerieMax;
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

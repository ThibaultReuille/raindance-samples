#include <raindance/Raindance.hh>
#include <raindance/Core/Camera/Camera.hh>
#include <raindance/Core/Primitives/Quad.hh>
#include <raindance/Core/Primitives/WideLine.hh>
#include <raindance/Core/Primitives/Grid.hh>
#include <raindance/Core/Transformation.hh>
#include <raindance/Core/Icon.hh>

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
        m_Size = 1.0;

        m_Begin = 0;
        m_End = 0;

        m_WideLineShader = ResourceManager::getInstance().loadShader("shaders/wideline",
            Resources_Shaders_Primitives_wideline_vert, sizeof(Resources_Shaders_Primitives_wideline_vert),
            Resources_Shaders_Primitives_wideline_frag, sizeof(Resources_Shaders_Primitives_wideline_frag));
        // m_WideLineShader->dump();
    }

    virtual ~TimeSerie()
    {
        ResourceManager::getInstance().unload(m_WideLineShader);
    }

    virtual void draw(Context& context, Camera& camera)
    {
        glEnable(GL_BLEND);
        glBlendFunc (GL_SRC_ALPHA, GL_DST_ALPHA);

        drawLines(context, camera);
    }

    virtual void drawLines(Context& context, Camera& camera)
    {
        m_WideLineShader->use();
        
        m_WideLineShader->uniform("u_ModelViewProjection").set(camera.getViewProjectionMatrix());
        // m_WideLineShader->uniform("u_ExtrudeDirection").set(glm::vec3(0.0, 1.0, 0.0));
        // m_WideLineShader->uniform("u_Texture").set(Icon->getTexture(m_TextureID));

        size_t index = m_Begin;

        glm::vec3 lastPos;
        glm::vec3 currPos = m_Ticks[index].data();
        glm::vec4 color = glm::vec4(1.2, 1.2, 1.2, 0.8);

        glm::vec3 firstPos = glm::vec3(currPos.x, 0.0, 0.0);
        
        do
        {
            index = (index + 1) % m_Ticks.size();
            if (index == m_End)
                break;

            lastPos = currPos;
            currPos = m_Ticks[index].data();

            if (true)
            {
                m_WideLineShader->uniform("u_Mode").set(0.0f);
                m_WideLineShader->uniform("u_StartPosition").set(lastPos - firstPos);
                m_WideLineShader->uniform("u_EndPosition").set(currPos - firstPos);
                m_WideLineShader->uniform("u_Tint").set(color);

                context.geometry().bind(m_WideLine.getVertexBuffer(), *m_WideLineShader);
                context.geometry().drawArrays(GL_LINES, 0, 2 * sizeof(WideLine::Vertex));
                context.geometry().unbind(m_WideLine.getVertexBuffer());
            }
            else
            {
                glm::vec3 extrusion = 2.0f * m_Size * glm::vec3(0.0, 1.0, 0.0);

                m_WideLineShader->uniform("u_Mode").set(3.0f);
                m_WideLineShader->uniform("u_StartPosition").set(lastPos);
                m_WideLineShader->uniform("u_EndPosition").set(currPos);
                m_WideLineShader->uniform("u_ExtrudeDirection").set(extrusion);
                m_WideLineShader->uniform("u_Tint").set(color);

                context.geometry().bind(m_WideLine.getVertexBuffer(), *m_WideLineShader);
                context.geometry().drawArrays(GL_TRIANGLE_STRIP, 0, m_WideLine.getVertexBuffer().size() / sizeof(WideLine::Vertex));
                context.geometry().unbind(m_WideLine.getVertexBuffer());
            }
        } while (true);
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

    inline void setSize(float size) { m_Size = size; }

protected:
    std::vector<Tick> m_Ticks;
    size_t m_Begin;
    size_t m_End;

    Quad* m_Quad;
    WideLine m_WideLine;
    Shader::Program* m_WideLineShader;
    float m_Size;

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

        SAFE_DELETE(m_Grid);
    }

    virtual void initialize()
    {
        float width = static_cast<float>(glutGet(GLUT_WINDOW_WIDTH));
        float height = static_cast<float>(glutGet(GLUT_WINDOW_HEIGHT));

        m_Camera.setOrthographicProjection(0, width, - height / 2, height / 2, - 10.0, 10.0);

        m_Camera.lookAt(glm::vec3(0, 0, 1.0), glm::vec3(0, 0, 0), glm::vec3(0, 1, 0));

        m_TimeSerie = new TimeSerie(600, glm::vec4(WHITE, 1.0));
        m_TimeSerie->setSize(2.0);
        m_TimeSerieAvg = new TimeSerie(600, glm::vec4(LOVE_RED, 1.0));
        m_TimeSerieMin = new TimeSerie(600, glm::vec4(GUNMETAL, 1.0));
        m_TimeSerieMax = new TimeSerie(600, glm::vec4(GUNMETAL, 1.0));

        Grid::Parameters params = {};

        params.Dimension = glm::vec2(600, 200);
        params.Origin = glm::vec2(0, -100);
        params.Step = glm::vec2(100.0, 100.0);
        params.Division = glm::vec2(10.0, 10.0);

        params.Color = glm::vec4(0.02, 0.15, 0.4, 1.0);
        params.BackgroundColor = 0.5f * params.Color;
        
        m_Grid = new Grid(params);

        glClearColor(0.1, 0.1, 0.1, 1.0);
        glDisable(GL_DEPTH_TEST);
    }

    virtual void destroy()
    {
    }

    virtual void draw()
    {
        glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

        m_Grid->draw(context(), m_Camera);

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
        static unsigned int _count = 0;

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

        if (t - _lastTime >= 0.030f)
        {
            float tscale = static_cast<float>(_count);

            LOG("%f\t%f\n", tscale, _value);

            m_TimeSerie->push(TimeSerie::Tick(glm::vec3(tscale, _value, 0.0)));

            m_TimeSerie->getStats(50, &moving_avg, &moving_min, &moving_max);
            m_TimeSerieAvg->push(TimeSerie::Tick(glm::vec3(tscale, moving_avg, 0.0)));
            m_TimeSerieMin->push(TimeSerie::Tick(glm::vec3(tscale, moving_min, 0.0)));
            m_TimeSerieMax->push(TimeSerie::Tick(glm::vec3(tscale, moving_max, 0.0)));

            _value += RANDOM_FLOAT(-10.0, 10.0);
            _value = std::max(min, std::min(max, _value));
            _lastTime = t;     
            _count ++;

            if (_count >= 600)
            m_Grid->parameters().Shift.x += 1.0;
        }

        glutPostRedisplay();
    }

private:
    Camera m_Camera;
    TimeSerie* m_TimeSerie;
    TimeSerie* m_TimeSerieAvg;
    TimeSerie* m_TimeSerieMin;
    TimeSerie* m_TimeSerieMax;

    Grid* m_Grid;
};

int main(int argc, char** argv)
{
    Demo demo;

    demo.create(argc, argv);

    // demo.addWindow("Stream", glutGet(GLUT_SCREEN_WIDTH), glutGet(GLUT_SCREEN_HEIGHT));
    demo.addWindow("Stream", 600, 300);

    demo.initialize();
    demo.run();

    demo.destroy();
}

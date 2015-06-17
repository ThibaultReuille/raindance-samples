#include <raindance/Raindance.hh>
#include <raindance/Core/Camera/Camera.hh>
#include <raindance/Core/Primitives/Quad.hh>
#include <raindance/Core/Primitives/WideLine.hh>
#include <raindance/Core/Primitives/Grid.hh>
#include <raindance/Core/Transformation.hh>
#include <raindance/Core/Icon.hh>

#include <raindance/Core/Primitives/Polyline.hh>

class TimeSerie
{
public:

    TimeSerie(size_t size, const glm::vec4& color)
    {
        m_Values.resize(size);
        m_Begin = 0;
        m_End = 0;

        m_Polyline = new Polyline();
        m_Polyline->setColor(color);
    }

    virtual ~TimeSerie()
    {
        SAFE_DELETE(m_Polyline);
    }

    virtual void draw(Context* context, Camera& camera)
    {
        glEnable(GL_BLEND);
        glBlendFunc (GL_SRC_ALPHA, GL_DST_ALPHA);

        Transformation transformation;

        m_Polyline->draw(*context, camera, transformation, m_Values, m_Begin, m_End);
    }

    virtual void push(float timestamp, float value)
    {
        Polyline::Vertex v = {};
        v.Position = glm::vec3(timestamp, value, 0.0);
  
        m_Values[m_End] = v;
        m_End = (m_End + 1) % m_Values.size();

        if (m_End == m_Begin)
            m_Begin = (m_Begin + 1) % m_Values.size();
    }

    virtual void getStats(size_t n, float* average, float* min, float* max)
    {
        if (n > m_Values.size())
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
                index = m_Values.size();
            index--;

            value = m_Values[index].Position.y;

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

    inline void setWidth(float value) { m_Polyline->setWidth(value); }
    inline float getWidth() { return m_Polyline->getWidth(); }

protected:
    size_t m_Begin;
    size_t m_End;
    std::vector<Polyline::Vertex> m_Values;
    Polyline* m_Polyline;
};

class DemoWindow : public rd::Window
{
public:
    DemoWindow(rd::Window::Settings* settings)
    : Window(settings)
    {
        float width = static_cast<float>(settings->Width);
        float height = static_cast<float>(settings->Height);

        m_Camera.setOrthographicProjection(0, width, - height / 2, height / 2, - 10.0, 10.0);
        m_Camera.lookAt(glm::vec3(0, 0, 1.0), glm::vec3(0, 0, 0), glm::vec3(0, 1, 0));

        {
            m_TimeSerie = new TimeSerie(600, glm::vec4(WHITE, 1.0));
            m_TimeSerieAvg1 = new TimeSerie(600, glm::vec4(LOVE_RED, 1.0));
            m_TimeSerieAvg2 = new TimeSerie(600, glm::vec4(SKY_BLUE, 1.0));
            m_TimeSerieMin = new TimeSerie(600, glm::vec4(GUNMETAL, 1.0));
            m_TimeSerieMax = new TimeSerie(600, glm::vec4(GUNMETAL, 1.0));
        }

        {
            Grid::Parameters params = {};

            params.Dimension = glm::vec2(600, 200);
            //params.Origin = glm::vec2(0, -100);
            params.Step = glm::vec2(100.0, 100.0);
            params.Division = glm::vec2(10.0, 10.0);

            params.Color = glm::vec4(0.02, 0.15, 0.4, 1.0);
            //params.BackgroundColor = 0.5f * params.Color;
            
            m_Grid = new Grid(params);
        }

        glClearColor(0.1, 0.1, 0.1, 1.0);
        glDisable(GL_DEPTH_TEST);
    }

    virtual ~DemoWindow()
    {
        SAFE_DELETE(m_TimeSerie);
        SAFE_DELETE(m_TimeSerieAvg1);
        SAFE_DELETE(m_TimeSerieAvg2);
        SAFE_DELETE(m_TimeSerieMin);
        SAFE_DELETE(m_TimeSerieMax);

        SAFE_DELETE(m_Grid);
    }
    
    virtual void initialize(Context* context)
    {
        (void) context;
    }

    virtual void draw(Context* context)
    {
        Transformation transformation;

        glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

        transformation.push();
        transformation.translate(glm::vec3(0, -100, 0));
        m_Grid->draw(context, m_Camera, transformation);
        transformation.pop();

        m_TimeSerie->draw(context, m_Camera);
        m_TimeSerieAvg1->draw(context, m_Camera);
        m_TimeSerieAvg2->draw(context, m_Camera);
        m_TimeSerieMin->draw(context, m_Camera);
        m_TimeSerieMax->draw(context, m_Camera);
    }

    virtual void idle(Context* context)
    {
        (void) context;

        static bool _first = true;
        static float _lastTime;
        static float _value = 0.0;
        static unsigned int _count = 0;

        if (_first)
        {
            context->clock().start();
            _lastTime = context->clock().seconds();
            _first = false;
        }

        float t = context->clock().seconds();

        float min = -100.0f;
        float max = +100.0f;

        float moving_min;
        float moving_max;
        float moving_avg;

        if (t - _lastTime >= 0.030f)
        {
            float tscale = static_cast<float>(_count);

            LOG("%f\t%f\n", tscale, _value);

            m_TimeSerie->push(tscale, _value);

            m_TimeSerie->getStats(25, &moving_avg, &moving_min, &moving_max);
            m_TimeSerieAvg1->push(tscale, moving_avg);
            m_TimeSerieMin->push(tscale, moving_min);
            m_TimeSerieMax->push(tscale, moving_max);

            m_TimeSerie->getStats(50, &moving_avg, &moving_min, &moving_max);
            m_TimeSerieAvg2->push(tscale, moving_avg);    

            _value += RANDOM_FLOAT(-10.0, 10.0);
            _value = std::max(min, std::min(max, _value));
            _lastTime = t;     
            _count ++;

            if (_count >= 600)
            m_Grid->parameters().Shift.x += 1.0;
        }
    }

private:
    Camera m_Camera;
    TimeSerie* m_TimeSerie;
    TimeSerie* m_TimeSerieAvg1;
    TimeSerie* m_TimeSerieAvg2;
    TimeSerie* m_TimeSerieMin;
    TimeSerie* m_TimeSerieMax;

    Grid* m_Grid;
};

int main(int argc, char** argv)
{
    auto demo = new Raindance(argc, argv);

    rd::Window::Settings settings;
    settings.Title = std::string("Stream");
    settings.Width = 600;
    settings.Height = 200;

    demo->add(new DemoWindow(&settings));
    demo->run();

    delete demo;
}

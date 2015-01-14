#include <raindance/Raindance.hh>
#include <raindance/Core/Camera/Camera.hh>
#include <raindance/Core/Transformation.hh>
#include <raindance/Core/Charts/LineChart.hh>
#include <raindance/Core/Charts/HeightMap.hh>
#include <raindance/Core/Charts/IconMap.hh>

class DemoWindow : public Window
{
public:
    
    DemoWindow(const char* title, int width, int height, bool fullscreen = false)
    : Window(title, width, height, fullscreen)
    {
        m_IconMap = NULL;
        m_LineChart1 = NULL;
        m_LineChart2 = NULL;
        m_HeightMap = NULL;

        m_Camera2D.setOrthographicProjection(0, 1024, 0, 728, 0.1f, 1024.0f);
        m_Camera2D.lookAt(glm::vec3(0.0, 0.0, 5.0), glm::vec3(0, 0, 0), glm::vec3(0, 1, 0));

        auto viewport = this->getViewport();
        m_Camera3D.setPerspectiveProjection(60.0f, viewport.getDimension()[0] / viewport.getDimension()[1], 0.1f, 1024.0f);
        m_Camera3D.lookAt(glm::vec3(-50.0, 30.0, -50.0), glm::vec3(0, 0, 0), glm::vec3(0, 1, 0));

        {
            m_LineChart1 = new LineChart(glm::vec2(0, 100), glm::vec2(0, 10));
            m_LineChart1->setTitle("Random()");
            m_LineChart1->setBackgroundColor(glm::vec4(HEX_COLOR(0x222222), 1.0));
            m_LineChart1->setBorderColor(glm::vec4(HEX_COLOR(0x0D4F8B), 1.0));

            auto rand1 = m_LineChart1->addGraph("Random 1", glm::vec4(HEX_COLOR(0xFF9097), 1.0));
            for (float t = 0; t <= 100; t += 2.0)
                rand1->addPoint(glm::vec2(t, RANDOM_FLOAT(0, 10)));

            auto rand2 = m_LineChart1->addGraph("Random 2", glm::vec4(HEX_COLOR(0x5FA6CC), 1.0));
            for (float t = 0; t <= 100; t += 10.0)
                rand2->addPoint(glm::vec2(t, RANDOM_FLOAT(0, 10)));

            m_LineChart1->update();
        }
        {
            m_LineChart2 = new LineChart(glm::vec2(-50, 50), glm::vec2(0, 10));
            m_LineChart2->setTitle("Gaussian Distributions");
            m_LineChart2->setBackgroundColor(glm::vec4(HEX_COLOR(0x222222), 1.0));
            m_LineChart2->setBorderColor(glm::vec4(HEX_COLOR(0x0D4F8B), 1.0));

            auto rand1 = m_LineChart2->addGraph("Gaussian 1", glm::vec4(HEX_COLOR(0xF7FF73), 1.0));
            for (float t = -50; t <= 50; t += 2.0)
                rand1->addPoint(glm::vec2(t, 10 * gaussian(t / 10, 2.0, 1.0)));

            auto rand2 = m_LineChart2->addGraph("Gaussian 2", glm::vec4(HEX_COLOR(0xFF6159), 1.0));
            for (float t = -50; t <= 50; t += 2.0)
                rand2->addPoint(glm::vec2(t, 10 * gaussian(t / 10, 0, 0.5)));

            auto rand3 = m_LineChart2->addGraph("Gaussian 3", glm::vec4(HEX_COLOR(0x73FFF6), 1.0));
            for (float t = -50; t <= 50; t += 2.0)
                rand3->addPoint(glm::vec2(t, 10 * gaussian(t / 10, -2.0, 0.7)));

            m_LineChart2->update();
        }
        {
            m_HeightMap = new HeightMap(50, 50);

            for (unsigned int i = 0; i < 50; i++)
                for (unsigned int j = 0; j < 50; j++)
                {
                    float x = 2 * M_PI * static_cast<float>(i) / 50.0 - M_PI;
                    float y = 2 * M_PI * static_cast<float>(j) / 50.0 - M_PI;
                    m_HeightMap->setValue(i, j, sin(x * x) + cos(y * y) + RANDOM_FLOAT(-0.25, 0.25));
                }
            m_HeightMap->update();
        }
        {
            const unsigned long size = 777;
            char* memory = new char[size];

            m_IconMap = new IconMap(16, size / 4 + 4);

            for (unsigned long i = 0; i < size; i++)
            {
                float v = 0.4 + 0.7 * static_cast<float>(memory[i]) / 255.0;
                glm::vec4 color = glm::vec4(v / 2, v, v, 1.0);
                m_IconMap->set(i % m_IconMap->width(), i / m_IconMap->width(), 1, color, 1.0);
            }

            delete[] memory;
        }

        glClearColor(0.2, 0.2, 0.2, 1.0);
        glEnable(GL_DEPTH_TEST);
    }

    virtual ~DemoWindow()
    {
        SAFE_DELETE(m_LineChart1);
        SAFE_DELETE(m_LineChart2);
        SAFE_DELETE(m_HeightMap);
        SAFE_DELETE(m_IconMap);
    }
    
    virtual void initialize(Context* context)
    {
        (void) context;
    }

    virtual void draw(Context* context)
    {
        auto viewport = this->getViewport();

        glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

        Transformation transformation;

        transformation.push();
        transformation.translate(glm::vec3(-25, 15, -25));
        m_HeightMap->draw(*context, transformation.state(), m_Camera3D.getViewMatrix(), m_Camera3D.getProjectionMatrix());
        transformation.pop();

        glClear(GL_DEPTH_BUFFER_BIT);

        transformation.push();
        transformation.translate(glm::vec3(10, 10, 0.0));
        transformation.scale(glm::vec3(10, 10, 1));
        m_LineChart1->draw(*context, transformation.state(), m_Camera2D.getViewMatrix(), m_Camera2D.getProjectionMatrix());
        transformation.pop();

        transformation.push();
        transformation.translate(glm::vec3(10, 120, 0.0));
        transformation.scale(glm::vec3(10, 10, 1));
        m_LineChart2->draw(*context, transformation.state(), m_Camera2D.getViewMatrix(), m_Camera2D.getProjectionMatrix());
        transformation.pop();

        transformation.push();
        transformation.translate(glm::vec3(10, viewport.getDimension()[1] - 10, 0.0));
        transformation.scale(glm::vec3(10, 10, 1));
        m_IconMap->draw(*context, transformation.state(), m_Camera2D.getViewMatrix(), m_Camera2D.getProjectionMatrix());

        transformation.pop();
    }

    virtual void idle(Context* context)
    {
        (void) context;

        float t = 0.1f * static_cast<float>(m_Clock.milliseconds()) / 1000.0f;
        m_Camera3D.lookAt(glm::vec3(50 * cos(t), 25, 50 * sin(t)), glm::vec3(0, 0, 0), glm::vec3(0, 1, 0));
    }

    inline float gaussian(float x, float mu, float sigma)
    {
        return (1.0 / (sigma * sqrt(2 * M_PI))) * exp(- ((x - mu) * (x - mu)) / (2 * sigma * sigma));
    }

private:
    Clock m_Clock;

    Camera m_Camera2D;
    Camera m_Camera3D;

    LineChart* m_LineChart1;
    LineChart* m_LineChart2;
    HeightMap* m_HeightMap;
    IconMap* m_IconMap;
};

int main(int argc, char** argv)
{
    auto demo = new Raindance(argc, argv);
    demo->add(new DemoWindow("Charts", 1024, 728));
    demo->run();
    delete demo;
}

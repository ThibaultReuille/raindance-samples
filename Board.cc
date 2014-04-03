#include <raindance/Raindance.hh>
#include <raindance/Core/Camera/Camera.hh>
#include <raindance/Core/Transformation.hh>
#include <raindance/Core/Primitives/Grid.hh>

class Demo : public RainDance
{
public:
    Demo()
    {
        m_Grid = NULL;
    }

    virtual ~Demo()
    {
    }

    virtual void initialize()
    {
        m_Camera2D.setOrthographicProjection(0, 1024, 0, 728, 0.1f, 1024.0f);
        m_Camera2D.lookAt(glm::vec3(0.0, 0.0, 5.0), glm::vec3(0, 0, 0), glm::vec3(0, 1, 0));

        m_Camera3D.setPerspectiveProjection(60.0f, (float)m_Window->width() / (float)m_Window->height(), 0.1f, 1024.0f);
        m_Camera3D.lookAt(glm::vec3(-50.0, 30.0, -50.0), glm::vec3(0, 0, 0), glm::vec3(0, 1, 0));

        m_Grid = new Grid(100, 100);
        m_Grid->setColor(glm::vec4(0.5, 0.5, 0.5, 1.0));

        glClearColor(0.2, 0.2, 0.2, 1.0);
        glEnable(GL_DEPTH_TEST);
    }

    virtual void destroy()
    {
    }

    virtual void draw()
    {
        glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

        Transformation transformation;

        transformation.push();
        transformation.rotate(90, glm::vec3(1.0, 0, 0));
        transformation.translate(glm::vec3(-50, -50, 0));
        m_Grid->draw(m_Context, transformation.state(), m_Camera3D.getViewMatrix(), m_Camera3D.getProjectionMatrix());
        transformation.pop();

        finish();
    }

    virtual void idle()
    {
        float t = 0.1f * static_cast<float>(m_Clock.milliseconds()) / 1000.0f;
        m_Camera3D.lookAt(glm::vec3(50 * cos(t), 25, 50 * sin(t)), glm::vec3(0, 0, 0), glm::vec3(0, 1, 0));

        glutPostRedisplay();
    }

private:
    Clock m_Clock;

    Camera m_Camera2D;
    Camera m_Camera3D;

    Grid* m_Grid;
};

int main(int argc, char** argv)
{
    Demo demo;
    demo.create(argc, argv);
    demo.addWindow("Board", 1024, 728);
    demo.initialize();
    demo.run();
    demo.destroy();
}

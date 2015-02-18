#include <raindance/Raindance.hh>
#include <raindance/Core/Camera/Camera.hh>
#include <raindance/Core/Primitives/Cube.hh>
#include <raindance/Core/Transformation.hh>
#include <raindance/Core/FS.hh>
#include <raindance/Core/Icon.hh>

int g_ParticleCount = 0;

class Mesh
{
public:

    struct Vertex
    {
        glm::vec3 Position;
        glm::vec2 UV;
        // glm::vec3 Normal;
    };

    struct Instance
    {
        glm::vec3 Translation;
        glm::vec3 Scale;
        glm::vec4 Color;
    };

    Mesh()
    {
        FS::TextFile vert("Assets/mesh_instanced.vert");
        FS::TextFile frag("Assets/mesh_instanced.frag");

        m_Icon = new Icon();
        m_Icon->load("particles/1", FS::BinaryFile("Assets/mesh_particle.png"));

        m_Shader = ResourceManager::getInstance().loadShader("mesh_instanced", vert.content(), frag.content());
        m_Shader->dump();

        {
            m_VertexBuffer << glm::vec3(-0.5, -0.5, 0.0) << glm::vec2(0, 1);
            m_VertexBuffer << glm::vec3( 0.5, -0.5, 0.0) << glm::vec2(1, 1);
            m_VertexBuffer << glm::vec3(-0.5,  0.5, 0.0) << glm::vec2(0, 0);
            m_VertexBuffer << glm::vec3( 0.5,  0.5, 0.0) << glm::vec2(1, 0);
        
            m_VertexBuffer.describe("a_Position", 3, GL_FLOAT, sizeof(Vertex), 0);
            m_VertexBuffer.describe("a_UV", 2, GL_FLOAT, sizeof(Vertex), sizeof(glm::vec3));
     
            m_VertexBuffer.generate(Buffer::STATIC);
        }
        {
            for (int n = 0; n < g_ParticleCount; n++)
            {
                float d = 50;

                Instance i;

                i.Translation = d * glm::vec3(
                    RANDOM_FLOAT(-1.0, 1.0),
                    RANDOM_FLOAT(-1.0, 1.0),
                    RANDOM_FLOAT(-1.0, 1.0));
                
                float s = RANDOM_FLOAT(0.5, 2.0);
                i.Scale = glm::vec3(s, s, 1.0);
                
                i.Color = glm::vec4(
                    RANDOM_FLOAT(0.0, 1.0),
                    RANDOM_FLOAT(0.0, 1.0),
                    RANDOM_FLOAT(0.0, 1.0),
                    1.0);

                m_InstanceBuffer.push(&i, sizeof(Instance));
            }

            m_InstanceBuffer.describe("a_Translation", 3, GL_FLOAT, sizeof(Instance), 0);
            m_InstanceBuffer.describe("a_Scale", 3, GL_FLOAT, sizeof(Instance), sizeof(glm::vec3));
            m_InstanceBuffer.describe("a_Color", 4, GL_FLOAT, sizeof(Instance), 2 * sizeof(glm::vec3));

            m_InstanceBuffer.generate(Buffer::STATIC);
        }
    }

    virtual ~Mesh()
    {
        SAFE_DELETE(m_Icon);
        ResourceManager::getInstance().unload(m_Shader);
    }

    virtual void initialize(Context* context)
    {
        (void) context;
    }

    virtual void draw(Context* context, Camera& camera, Transformation& transformation)
    {
        glDisable(GL_DEPTH_TEST);
        glEnable(GL_BLEND);
        glBlendFunc(GL_SRC_ALPHA, GL_DST_ALPHA);
        
        m_Shader->use();
        m_Shader->uniform("u_Texture").set(m_Icon->getTexture(0));

        m_Shader->uniform("u_ModelViewMatrix").set(camera.getViewMatrix() * transformation.state());
        m_Shader->uniform("u_ProjectionMatrix").set(camera.getProjectionMatrix());

        context->geometry().bind(m_VertexBuffer, *m_Shader);        
        context->geometry().bind(m_InstanceBuffer, *m_Shader);
        
        glVertexAttribDivisorARB(m_Shader->attribute("a_Position").location(), 0); // Same vertices per instance

        glVertexAttribDivisorARB(m_Shader->attribute("a_Translation").location(), 1);
        glVertexAttribDivisorARB(m_Shader->attribute("a_Scale").location(), 1);
        glVertexAttribDivisorARB(m_Shader->attribute("a_Color").location(), 1);

        context->geometry().drawArraysInstanced(GL_TRIANGLE_STRIP, 0, m_VertexBuffer.size() / sizeof(Vertex), m_InstanceBuffer.size() / sizeof(Instance));
        
        context->geometry().unbind(m_VertexBuffer);
        context->geometry().unbind(m_InstanceBuffer);
    }

private:
    Shader::Program* m_Shader;
    Icon* m_Icon;
    Buffer m_VertexBuffer;
    Buffer m_InstanceBuffer;
};

class DemoWindow : public Window
{
public:
    DemoWindow(const char* title, int width, int height, bool fullscreen = false)
    : Window(title, width, height, fullscreen)
    {
        auto viewport = this->getViewport();
        m_Camera.setPerspectiveProjection(60.0f, viewport.getDimension()[0] / viewport.getDimension()[1], 0.1f, 1024.0f);
        m_Camera.lookAt(glm::vec3(1.5, 2, 2.5), glm::vec3(0, 0, 0), glm::vec3(0, 1, 0));

        m_Mesh = new Mesh();

        glClearColor(0.2, 0.2, 0.2, 1.0);
        glEnable(GL_DEPTH_TEST);
    }

    virtual ~DemoWindow()
    {
        delete m_Mesh;
    }

    virtual void draw(Context* context)
    {
        glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

        Transformation transformation;

        m_Mesh->draw(context, m_Camera, transformation);
    }

    virtual void idle(Context* context)
    {
        float t = context->clock().seconds() / 4;
        float r = 125 + 25 * cos(t / 3);

        glm::vec3 pos = glm::vec3(r * cos(t), r * cos(t / 2), r * sin(t));

        m_Camera.lookAt(pos, glm::vec3(0.0, 0.0, 0.0), glm::vec3(0.0, 1.0, 0.0));
    }

private:
    Camera m_Camera;
    Mesh* m_Mesh;
};

int main(int argc, char** argv)
{
    if (argc != 2)
    {
        LOG("Usage: %s <number of particles>\n", argv[0]);
        return 0;
    }

    g_ParticleCount = std::stoi(std::string(argv[1]));

    auto demo = new Raindance(argc, argv);

    demo->add(new DemoWindow("Mesh", 800, 600));

    demo->run();

    delete demo;
}

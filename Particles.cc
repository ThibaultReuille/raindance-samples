#include <raindance/Raindance.hh>
#include <raindance/Core/Camera/Camera.hh>
#include <raindance/Core/Primitives/Cube.hh>
#include <raindance/Core/Transformation.hh>
#include <raindance/Core/FS.hh>
#include <raindance/Core/Icon.hh>

#include <raindance/Core/OpenCL.hh>

int g_ParticleCount = 0;

class Particles
{
public:

    struct Particle
    {
        glm::vec3 Position;
        glm::vec2 UV;
        // glm::vec3 Normal;
    };

    struct Instance
    {
        glm::vec4 Translation;
        glm::vec4 Scale;
        glm::vec4 Color;
    };

    Particles()
    {
        FS::TextFile vert("Assets/particles_instanced.vert");
        FS::TextFile frag("Assets/particles_instanced.frag");

        m_Icon = new Icon();
        m_Icon->load("Particles/icon", FS::BinaryFile("Assets/particles_icon.png"));

        m_Shader = ResourceManager::getInstance().loadShader("mesh_instanced", vert.content(), frag.content());
        m_Shader->dump();

        {
            m_VertexBuffer << glm::vec3(-0.5, -0.5, 0.0) << glm::vec2(0, 1);
            m_VertexBuffer << glm::vec3( 0.5, -0.5, 0.0) << glm::vec2(1, 1);
            m_VertexBuffer << glm::vec3(-0.5,  0.5, 0.0) << glm::vec2(0, 0);
            m_VertexBuffer << glm::vec3( 0.5,  0.5, 0.0) << glm::vec2(1, 0);
        
            m_VertexBuffer.describe("a_Position", 3, GL_FLOAT, sizeof(Particle), 0);
            m_VertexBuffer.describe("a_UV", 2, GL_FLOAT, sizeof(Particle), sizeof(glm::vec3));
     
            m_VertexBuffer.generate(Buffer::STATIC);
        }

        {
            for (int n = 0; n < g_ParticleCount; n++)
            {
                float d = 200;

                Instance i;

                i.Translation = d * glm::vec4(
                    RANDOM_FLOAT(-1.0, 1.0),
                    RANDOM_FLOAT(-1.0, 1.0),
                    0.0,
                    1.0);
                
                float s = RANDOM_FLOAT(0.1, 2.0);
                i.Scale = glm::vec4(s, s, 1.0, 1.0);
                
                i.Color = glm::vec4(
                    RANDOM_FLOAT(0.0, 1.0),
                    RANDOM_FLOAT(0.0, 1.0),
                    RANDOM_FLOAT(0.0, 1.0),
                    1.0);

                m_InstanceBuffer.push(&i, sizeof(Instance));
            }

            m_InstanceBuffer.describe("a_Translation", 4, GL_FLOAT, sizeof(Instance), 0);
            m_InstanceBuffer.describe("a_Scale", 4, GL_FLOAT, sizeof(Instance), 1 * sizeof(glm::vec4));
            m_InstanceBuffer.describe("a_Color", 4, GL_FLOAT, sizeof(Instance), 2 * sizeof(glm::vec4));

            m_InstanceBuffer.generate(Buffer::STATIC);
        }
    }

    virtual ~Particles()
    {
        SAFE_DELETE(m_Icon);
        ResourceManager::getInstance().unload(m_Shader);
    }

    void initialize(Context* context)
    {
        // OpenGL
        {
            m_Shader->use();
            m_Shader->uniform("u_Texture").set(m_Icon->getTexture(0));
        }

        // OpenCL
        {
            m_OpenCL.detect();
            //m_OpenCL.dump();

            // NOTE : We are assuming the last device is the best one.
            m_CL.Device = m_OpenCL.devices().back();

            m_OpenCL.dump(*m_CL.Device);

            m_CL.Context = m_OpenCL.createContext(*m_CL.Device);
            m_CL.Queue = m_OpenCL.createCommandQueue(*m_CL.Context);

            auto source = FS::TextFile("./Assets/particles_physics.cl");
         
            m_CL.Program = m_OpenCL.loadProgram(*m_CL.Context, "physics", source.content());
            m_CL.TestK = m_OpenCL.createKernel(*m_CL.Program, "sine_wave");

            m_CL.InstanceBuffer = m_OpenCL.createFromGLBuffer(*m_CL.Context, CL_MEM_READ_WRITE, m_InstanceBuffer.vbo());
        
            clFinish(m_CL.Queue->Object);
        }
    }

    void draw(Context* context, Camera& camera, Transformation& transformation)
    {
        glDisable(GL_DEPTH_TEST);
        glEnable(GL_BLEND);
        glBlendFunc(GL_SRC_ALPHA, GL_DST_ALPHA);
        
        m_Shader->use();

        m_Shader->uniform("u_ModelViewMatrix").set(camera.getViewMatrix() * transformation.state());
        m_Shader->uniform("u_ProjectionMatrix").set(camera.getProjectionMatrix());

        context->geometry().bind(m_VertexBuffer, *m_Shader);        
        context->geometry().bind(m_InstanceBuffer, *m_Shader);
        
        glVertexAttribDivisorARB(m_Shader->attribute("a_Position").location(), 0); // Same vertices per instance

        glVertexAttribDivisorARB(m_Shader->attribute("a_Translation").location(), 1);
        glVertexAttribDivisorARB(m_Shader->attribute("a_Scale").location(), 1);
        glVertexAttribDivisorARB(m_Shader->attribute("a_Color").location(), 1);

        context->geometry().drawArraysInstanced(GL_TRIANGLE_STRIP, 0, m_VertexBuffer.size() / sizeof(Particle), m_InstanceBuffer.size() / sizeof(Instance));
        
        context->geometry().unbind(m_VertexBuffer);
        context->geometry().unbind(m_InstanceBuffer);
    }

    void idle(Context* context)
    {
        size_t num_instances = m_InstanceBuffer.size() / sizeof(Instance);

        m_OpenCL.enqueueAcquireGLObjects(*m_CL.Queue, 1, &m_CL.InstanceBuffer->Object, 0, 0, NULL);

        {
            m_CL.Time = context->clock().seconds();

            m_CL.TestK->setArgument(0, *m_CL.InstanceBuffer);
            m_CL.TestK->setArgument(1, &m_CL.Time, sizeof(float));

            m_OpenCL.enqueueNDRangeKernel(*m_CL.Queue, *m_CL.TestK, 1, NULL, &num_instances, NULL, 0, NULL, NULL);
        }

        m_OpenCL.enqueueReleaseGLObjects(*m_CL.Queue, 1, &m_CL.InstanceBuffer->Object, 0, 0, NULL);
        clFinish(m_CL.Queue->Object);
    } 

private:
    Shader::Program* m_Shader;
    Icon* m_Icon;
    Buffer m_VertexBuffer;
    Buffer m_InstanceBuffer;

    struct OpenCLData
    {
        OpenCLData() {}

        OpenCL::Device* Device;
        OpenCL::Context* Context;
        OpenCL::CommandQueue* Queue;
        OpenCL::Program* Program;
        OpenCL::Kernel* TestK;

        OpenCL::Memory* InstanceBuffer;
        unsigned int Size;
        float Time;
    };

    OpenCL m_OpenCL;
    OpenCLData m_CL;    
};

class DemoWindow : public Window
{
public:
    DemoWindow(const char* title, int width, int height, bool fullscreen = false)
    : Window(title, width, height, fullscreen)
    {
        m_Camera.setOrthographicProjection(-width / 2, width / 2, -height / 2, height / 2, -1024.0, 1024.0);
        m_Camera.lookAt(glm::vec3(0.0, 0.0, -50), glm::vec3(0, 0, 0), glm::vec3(0, 1, 0));

        m_Particles = new Particles();

        glClearColor(0.2, 0.2, 0.2, 1.0);
        glEnable(GL_DEPTH_TEST);
    }

    virtual ~DemoWindow()
    {
        delete m_Particles;
    }

    void initialize(Context* context) override
    {
        m_Particles->initialize(context);
    }

    void draw(Context* context) override
    {
        glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

        Transformation transformation;

        m_Particles->draw(context, m_Camera, transformation);
    }

    void idle(Context* context) override
    {
        m_Particles->idle(context);
    }

private:
    Camera m_Camera;
    Particles* m_Particles;
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


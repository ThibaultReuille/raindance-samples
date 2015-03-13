#include <raindance/Raindance.hh>
#include <raindance/Core/Camera/Camera.hh>
#include <raindance/Core/Camera/Controllers.hh>

#include <raindance/Core/FS.hh>

#ifdef PARTICLES_COMMENT

class DemoWindow : public Window
{
public:

    struct ParticleInstance
    {
        glm::vec3 Position;
        glm::vec4 Color;
    };

    DemoWindow(const char* title, int width, int height, bool fullscreen = false)
    : Window(title, width, height, fullscreen)
    {
    }
    
    virtual ~DemoWindow()
    {
        ResourceManager::getInstance().unload(m_Shader);
    }

    virtual void initialize(Context* context)
    {
        m_NumParticles = 4096;
        m_NumForces = 4096;
        m_Time = 0.0;
        float volume = 10 * 10 * 10;
        m_K = pow(volume / m_NumParticles, 1.0 / 3.0);
        m_Iterations = 0;
        m_Temperature = 2.0;

        // Scene initialization
        {
            auto viewport = this->getViewport();
            m_Camera.setPerspectiveProjection(60.0f, viewport.getDimension()[0] / viewport.getDimension()[1], 0.1f, 1024.0f);
        
            m_SphericalCameraController.bind(context, &m_Camera);
            m_SphericalCameraController.setRadius(100);
            m_SphericalCameraController.updateCamera();

            auto vert = FS::TextFile("./Assets/particles.vert");
            auto frag = FS::TextFile("./Assets/particles.frag");
            auto physics = FS::TextFile("./Assets/particles_physics.vert");

            m_Shader = ResourceManager::getInstance().loadShader("particles", vert.content(), frag.content());
            m_Shader->dump();

            // Particle instancing
            float d = 50;
            ParticleInstance instance;

            instance.Color = glm::vec4(WHITE, 1.0);

            for (int i = 0; i < m_NumParticles)
            {
                float x = d * ((float) rand() / RAND_MAX - 0.5f);
                float y = d * ((float) rand() / RAND_MAX - 0.5f);
                float z = d * ((float) rand() / RAND_MAX - 0.5f);

                instance.Position = glm::vec4(x, y, z, 0.0);
                m_ParticleInstanceBuffer.push(&instance, sizeof(ParticleInstance));
            }

            m_ParticleInstanceBuffer.describe("a_Position", 3, GL_FLOAT, sizeof(ParticleInstance), 0);
            m_ParticleInstanceBuffer.describe("a_Color",    4, GL_FLOAT, sizeof(ParticleInstance), 3 * sizeof(GLfloat));
        }

        // OpenGL initialization
        {
            glClearColor(0.2, 0.2, 0.2, 1.0);
            glEnable(GL_DEPTH_TEST);
        }

        /*
        // OpenCL initialization
        {
            m_OpenCL.detect();
            m_OpenCL.dump();
            // TODO : Find best OpenCL architecture instead of hardcoded one
            // OpenCL::Device* device = m_OpenCL.getBestDevice();
            const OpenCL::Device* device = m_OpenCL.device(1);
            OpenCL::Context* context = m_OpenCL.createContext(*device);
            m_Queue = m_OpenCL.createCommandQueue(*context);

            OpenCL::Program* program = m_OpenCL.loadProgram(*context, "particles", g_ComputeProgram);
            m_RepulsionK = m_OpenCL.createKernel(*program, "repulsion");
            m_AttractionK = m_OpenCL.createKernel(*program, "attraction");
            m_MovementK = m_OpenCL.createKernel(*program, "movement");

            m_InputNodeBuffer = m_OpenCL.createBuffer(*context, CL_MEM_READ_ONLY, m_Nodes.size() * sizeof(Node));
            m_InputEdgeBuffer = m_OpenCL.createBuffer(*context, CL_MEM_READ_ONLY, m_Edges.size() * sizeof(Edge));
            m_ForceBuffer = m_OpenCL.createBuffer(*context, CL_MEM_READ_WRITE, m_Nodes.size() * sizeof(Node));
            m_OutputNodeBuffer = m_OpenCL.createBuffer(*context, CL_MEM_WRITE_ONLY, m_Nodes.size() * sizeof(Node));

            // Get the maximum work group size for executing the kernel on the device
            /*
            size_t local; // local domain size for our calculation
            error = clGetKernelWorkGroupInfo(m_OpenCL.kernel(m_KID).Object, m_OpenCL.device(did).ID, CL_KERNEL_WORK_GROUP_SIZE, sizeof(local), &local, NULL);
            if (error != CL_SUCCESS)
            {
                printf("[OpenCL] Error: Failed to retrieve kernel work group info! %d\n", error);
                throw;
            }
            LOG("LOCAL : %zu\n", local);
            *
        }
        */
    }

    virtual void draw(Context* context)
    {
        glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

        m_Shader->use();

        glm::mat4 model;

        model = glm::translate(glm::mat4(), glm::vec3(n.Position));
        m_Shader->uniform("u_ModelViewProjectionMatrix").set(m_Camera.getViewProjectionMatrix() * model);

        context->geometry().bind(m_ParticleInstanceBuffer, *m_Shader);
        context->geometry().drawArrays(GL_POINTS, 0, m_ParticleInstanceBuffer.size() / sizeof(ParticleInstance));
        context->geometry().unbind(m_ParticleInstanceBuffer);
    }

    virtual void idle(Context* context)
    {
        (void) context;
        
        /*
        if (m_Iterations == 0)
        {
            m_RepulsionK->setArgument(0, *m_InputNodeBuffer);
            m_RepulsionK->setArgument(1, *m_ForceBuffer);
            m_RepulsionK->setArgument(2, &m_NumParticles, sizeof(unsigned long));
            m_RepulsionK->setArgument(3, &m_K, sizeof(float));

            m_AttractionK->setArgument(0, *m_InputNodeBuffer);
            m_AttractionK->setArgument(1, *m_InputEdgeBuffer);
            m_AttractionK->setArgument(2, *m_ForceBuffer);
            m_AttractionK->setArgument(3, &m_NumForces, sizeof(unsigned long));
            m_AttractionK->setArgument(4, &m_K, sizeof(float));

            m_MovementK->setArgument(0, *m_InputNodeBuffer);
            m_MovementK->setArgument(1, *m_ForceBuffer);
            m_MovementK->setArgument(2, *m_OutputNodeBuffer);
            m_MovementK->setArgument(3, &m_Temperature, sizeof(float));

            m_Clock.reset();
        }

        m_OpenCL.enqueueWriteBuffer(*m_Queue, *m_InputNodeBuffer, CL_TRUE, 0, m_Nodes.size() * sizeof(Node), m_Nodes.data(), 0, NULL, NULL);
        m_OpenCL.enqueueWriteBuffer(*m_Queue, *m_InputEdgeBuffer, CL_TRUE, 0, m_Edges.size() * sizeof(Edge), m_Edges.data(), 0, NULL, NULL);

        m_OpenCL.enqueueNDRangeKernel(*m_Queue, *m_RepulsionK, 1, NULL, &m_NumParticles, NULL, 0, NULL, NULL);
        m_OpenCL.enqueueNDRangeKernel(*m_Queue, *m_AttractionK, 1, NULL, &m_NumForces, NULL, 0, NULL, NULL);
        m_OpenCL.enqueueNDRangeKernel(*m_Queue, *m_MovementK, 1, NULL, &m_NumParticles, NULL, 0, NULL, NULL);

        clFinish(m_Queue->Object);
        m_OpenCL.enqueueReadBuffer(*m_Queue, *m_OutputNodeBuffer, CL_TRUE, 0, m_Nodes.size() * sizeof(Node), m_Nodes.data(), 0, NULL, NULL);

        */

        m_Time = (float)m_Clock.milliseconds() / 1000.0f;
        m_Iterations++;
        if (m_Time > 0.0)
        {
            LOG("Iteration : %u, Time : %f, Iteration/sec : %f\n", m_Iterations, m_Time, m_Iterations / m_Time);
        }

        m_SphericalCameraController.updateCamera();
    }

    void onKey(int key, int scancode, int action, int mods) override
    {
        m_SphericalCameraController.onKey(key, scancode, action, mods);
    }

    void onScroll(double xoffset, double yoffset) override
    {
        m_SphericalCameraController.onScroll(xoffset, yoffset);
    }

    void onCursorPos(double xpos, double ypos) override
    {
        m_SphericalCameraController.onCursorPos(xpos, ypos);
    }

    void onMouseButton(int button, int action, int mods) override
    {
        m_SphericalCameraController.onMouseButton(button, action, mods);
    }

private:
    Clock m_Clock;
    Camera m_Camera;
    SphericalCameraController m_SphericalCameraController;
    SphereMesh* m_Sphere;
    Shader::Program* m_Shader;

    Buffer m_ParticleInstanceBuffer;

    unsigned long m_NumParticles;
    unsigned long m_NumForces;
    float m_K;
    float m_Time;
    unsigned int m_Iterations;
    float m_Temperature;
};

int main(int argc, char** argv)
{
    auto demo = new Raindance(argc, argv);
    demo->add(new DemoWindow("Particles", 1024, 728));
    demo->run();
    delete demo;
}

#endif

int main(int argc, char** argv)
{
    LOG("Disabled for now.\n");
}

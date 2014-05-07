#include <raindance/Raindance.hh>
#include <raindance/Core/Camera/Camera.hh>
#include <raindance/Core/Transformation.hh>
#include <raindance/Core/Primitives/Grid.hh>
#include <raindance/Core/Primitives/Cylinder.hh>
#include <raindance/Core/Light.hh>
#include <raindance/Core/Material.hh>

const std::string g_VertexShader = "                                                      \n\
    struct Light                                                                          \n\
    {                                                                                     \n\
        int Type;                                                                         \n\
        vec3 Position;                                                                    \n\
        vec3 Direction;                                                                   \n\
        vec3 Color;                                                                       \n\
    };                                                                                    \n\
                                                                                          \n\
    struct Material                                                                       \n\
    {                                                                                     \n\
        vec3 Ambient;                                                                     \n\
        vec4 Diffuse;                                                                     \n\
        vec3 Specular;                                                                    \n\
        float Shininess;                                                                  \n\
    };                                                                                    \n\
                                                                                          \n\
    attribute vec3 a_Position;                                                            \n\
    attribute vec3 a_Normal;                                                              \n\
    // attribute vec2 a_Texcoord;                                                         \n\
                                                                                          \n\
    uniform mat4 u_ModelMatrix;                                                           \n\
    uniform mat4 u_ViewMatrix;                                                            \n\
    uniform mat4 u_ProjectionMatrix;                                                      \n\
    uniform mat3 u_NormalMatrix;                                                          \n\
                                                                                          \n\
    uniform Light u_Light;                                                                \n\
    uniform Material u_Material;                                                          \n\
                                                                                          \n\
    varying vec3 v_Position;                                                              \n\
    // varying vec2 v_Texcoord;                                                           \n\
    varying vec3 v_Normal;                                                                \n\
    varying vec4 v_Color;                                                                 \n\
                                                                                          \n\
    void main(void)                                                                       \n\
    {                                                                                     \n\
        v_Position = vec3(u_ViewMatrix * u_ModelMatrix * vec4(a_Position, 1.0));          \n\
        v_Normal = normalize(u_NormalMatrix * a_Normal);                                  \n\
        // v_Texcoord = a_Texcoord;                                                       \n\
                                                                                          \n\
        v_Color = vec4(u_Material.Ambient, 0.0);                                          \n\
                                                                                          \n\
        vec3 lightPositionViewSpace = vec3(u_ViewMatrix * vec4(u_Light.Position, 0.0));   \n\
        vec3 lightDirection = normalize(lightPositionViewSpace - v_Position);             \n\
        float lambert = dot(v_Normal, lightDirection);                                    \n\
                                                                                          \n\
        if (lambert > 0.0)                                                                \n\
        {                                                                                 \n\
            v_Color += vec4(u_Light.Color, 0.0) * u_Material.Diffuse * lambert;           \n\
                                                                                          \n\
            vec3 eyeDirection = normalize(-v_Position);                                   \n\
            vec3 r = reflect(-lightDirection, v_Normal);                                  \n\
                                                                                          \n\
            float specular = pow(max(dot(r, eyeDirection), 0.0), u_Material.Shininess);   \n\
                                                                                          \n\
            v_Color += vec4(u_Material.Specular * specular, 0.0);                         \n\
        }                                                                                 \n\
                                                                                          \n\
        v_Color.a = u_Material.Diffuse.a;                                                 \n\
        gl_Position = u_ProjectionMatrix * vec4(v_Position, 1.0);                         \n\
    }                                                                                     \n\
";

const std::string g_FragmentShader = "  \n\
    #ifdef GL_ES                        \n\
    precision mediump float;            \n\
    #endif                              \n\
                                        \n\
    varying vec4 v_Color;               \n\
                                        \n\
    void main(void)                     \n\
    {                                   \n\
        gl_FragColor = v_Color;         \n\
    }                                   \n\
";

class Demo : public RainDance
{
public:
    struct Agent
    {
		glm::vec3 Position;	
        glm::vec4 Color;
        float Width;
		float Height;

		glm::vec3 Direction;
		float Speed;
		
		void randomizeColor()
		{
			Color.r = RANDOM_FLOAT(0.0, 1.0);
			Color.g = RANDOM_FLOAT(0.0, 1.0);
			Color.b = RANDOM_FLOAT(0.0, 1.0);
			Color.a = 1.0;
		}
		
		void randomizeDirection()
		{
			float alpha = RANDOM_FLOAT(0.0, 2 * M_PI);
			Direction = glm::vec3(2 * cos(alpha), 0, 2 * sin(alpha));
		}
    };

    Demo()
    {
        m_Grid = NULL;
        m_Agent = NULL;
        m_AgentShader = NULL;
    }

    virtual ~Demo()
    {
    }

    virtual void initialize()
    {
        m_Camera3D.setPerspectiveProjection(60.0f, (float)m_Window->width() / (float)m_Window->height(), 0.1f, 1024.0f);
        m_Camera3D.lookAt(glm::vec3(-50.0, 30.0, -50.0), glm::vec3(0, 0, 0), glm::vec3(0, 1, 0));

        m_Grid = new Grid(64, 64);
        m_Grid->setColor(glm::vec4(0.5, 0.5, 0.5, 1.0));

        float radius = 32;
        for (float alpha = 0; alpha < 2 * M_PI; alpha += 0.05)
        {
            auto agent = new Agent();

            agent->Position.x = RANDOM_FLOAT(0.1, 1.0) * radius * cos(alpha);
            agent->Position.y = 0;
            agent->Position.z = RANDOM_FLOAT(0.1, 1.0) * radius * sin(alpha);

			agent->Width = RANDOM_FLOAT(1.0, 2.0);
			agent->Height = RANDOM_FLOAT(1.0, 3.0);

			agent->randomizeColor();

			agent->randomizeDirection();
			agent->Speed = RANDOM_FLOAT(1.0, 5.0);

            m_Agents.push_back(agent);
        }

        m_Agent = new Cylinder(0.5, 0.5, 15, 10, glm::vec3(0, 0.5, 0));
        m_AgentShader = ResourceManager::getInstance().loadShader("Agents/Agent", g_VertexShader, g_FragmentShader);
        // m_AgentShader->dump();

        m_Light.setPosition(glm::vec3(128, 128, 0));
        m_Light.setColor(glm::vec3(1.2, 1.2, 1.2));

        glClearColor(0.2, 0.2, 0.2, 1.0);
        glEnable(GL_DEPTH_TEST);

		m_Clock.reset();
		m_LastTime = 0.0;
    }

    virtual void destroy()
    {
        SAFE_DELETE(m_Grid);
        ResourceManager::getInstance().unload(m_AgentShader);
    }

    virtual void reshape(int width, int height)
    {
        m_Camera3D.reshape(width, height);
    }

    virtual void draw()
    {
        glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

        Transformation transformation;

        transformation.push();
        transformation.rotate(90, glm::vec3(1.0, 0, 0));
        transformation.translate(glm::vec3(-(float) m_Grid->width() / 2, -(float) m_Grid->height() / 2, 0));
        m_Grid->draw(m_Context, transformation.state(), m_Camera3D.getViewMatrix(), m_Camera3D.getProjectionMatrix());
        transformation.pop();

        m_AgentShader->use();
        m_AgentShader->uniform("u_ProjectionMatrix").set(m_Camera3D.getProjectionMatrix());
        m_AgentShader->uniform("u_ViewMatrix").set(m_Camera3D.getViewMatrix());

        m_AgentShader->uniform("u_Light.Type").set(m_Light.getType());
        m_AgentShader->uniform("u_Light.Position").set(m_Light.getPosition());
        m_AgentShader->uniform("u_Light.Direction").set(m_Light.getDirection());
        m_AgentShader->uniform("u_Light.Color").set(m_Light.getColor());

        for (auto& agent : m_Agents)
        {
			transformation.push();
        	{
				transformation.translate(agent->Position);
            	transformation.scale(glm::vec3(agent->Width, agent->Height, agent->Width));
                        
                m_AgentShader->uniform("u_ModelMatrix").set(transformation.state());
                m_AgentShader->uniform("u_NormalMatrix").set(glm::transpose(glm::inverse(glm::mat3(m_Camera3D.getViewMatrix() * transformation.state()))));
                        
                m_AgentShader->uniform("u_Material.Ambient").set(m_Material.getAmbient());
                m_AgentShader->uniform("u_Material.Diffuse").set(agent->Color);
                m_AgentShader->uniform("u_Material.Specular").set(m_Material.getSpecular());
                m_AgentShader->uniform("u_Material.Shininess").set(m_Material.getShininess());
                        
                m_Context.geometry().bind(m_Agent->getVertexBuffer(), *m_AgentShader);
                m_Context.geometry().drawArrays(GL_TRIANGLE_STRIP, 0, m_Agent->getVertexBuffer().size() / sizeof(Cylinder::Vertex));
                m_Context.geometry().unbind(m_Agent->getVertexBuffer());
            }
		    transformation.pop();
        }

		finish();
	}

    virtual void idle()
    {
        float t = m_Clock.seconds();

        m_Camera3D.lookAt(glm::vec3(48 * cos(t / 10), 20, 48 * sin(t / 10)), glm::vec3(0, 0, 0), glm::vec3(0, 1, 0));

		glm::vec3 dim = glm::vec3((float)m_Grid->width(), 0, (float)m_Grid->height());

		float dt = t - m_LastTime;
		for (auto& agent : m_Agents)
		{
			glm::vec3 nextPosition = agent->Position + agent->Speed * dt * agent->Direction;

			if (nextPosition.x + agent->Width / 2 >= dim.x / 2 || nextPosition.x - agent->Width / 2 <= -dim.x / 2 ||
				nextPosition.z + agent->Width / 2 >= dim.z / 2 || nextPosition.z - agent->Width / 2 <= -dim.z / 2)
			{
				agent->randomizeDirection();
			}
			else
				agent->Position = nextPosition;
		}

		m_LastTime = t;
        glutPostRedisplay();
    }

private:
    Clock m_Clock;
	float m_LastTime;

    Camera m_Camera2D;
    Camera m_Camera3D;

    Light m_Light;
    Material m_Material;

    Grid* m_Grid;
    std::vector<Agent*> m_Agents;

    Cylinder* m_Agent;
    Shader::Program* m_AgentShader;
};

int main(int argc, char** argv)
{
    Demo demo;
    demo.create(argc, argv);
    demo.addWindow("Agents", 1024, 728);
    demo.initialize();
    demo.run();
    demo.destroy();
}

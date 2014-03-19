#include <raindance/Raindance.hh>
#include <raindance/Core/Sequencer.hh>

class TestSequence : public Sequence
{
public:
    TestSequence(const char* name, unsigned int timecode, unsigned int duration)
    : Sequence(name), m_Timecode(timecode), m_Duration(duration)
    {
    }

    virtual void start(Timecode timecode)
    {
        LOG("%lu > %s START\n", timecode, m_Name.c_str());
    }

    virtual Status play(Timecode timecode)
    {
        LOG("%lu > %s PLAY -- ", timecode, m_Name.c_str());

        if (m_Timecode <= timecode && timecode <= m_Timecode + m_Duration)
            LOG("OK\n");
        else
            LOG("KO\n");

        return LIVE;
    }

    virtual void stop(Timecode timecode)
    {
        LOG("%lu > %s STOP\n", timecode, m_Name.c_str());
    }

    unsigned int timecode() { return m_Timecode; }
    unsigned int duration() { return m_Duration; }

private:
    unsigned int m_Timecode;
    unsigned int m_Duration;
};

int main(int argc, char** argv)
{
    (void) argc;
    (void) argv;

    Track track("test");

    TestSequence* s;

    s = new TestSequence("A", 0,    1000); track.insert(s, s->timecode(), s->duration());
    s = new TestSequence("B", 1000, 1000); track.insert(s, s->timecode(), s->duration());
    s = new TestSequence("C", 2000,  500); track.insert(s, s->timecode(), s->duration());
    s = new TestSequence("D", 1500, 2000); track.insert(s, s->timecode(), s->duration());
    s = new TestSequence("E", 1400, 3000); track.insert(s, s->timecode(), s->duration());
    s = new TestSequence("F", 3000,  100); track.insert(s, s->timecode(), s->duration());
    s = new TestSequence("G", 1500,  100); track.insert(s, s->timecode(), s->duration());
    s = new TestSequence("H", 1500,  500); track.insert(s, s->timecode(), s->duration());

    track.dump();

    for (Timecode t = 0; t <= 5000; t += 200)
    {
        LOG("--- Timecode %lu\n", t);
        track.play(t);
    }
    return 0;
}

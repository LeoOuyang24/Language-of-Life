#ifndef CREATURES_H_INCLUDED
#define CREATURES_H_INCLUDED

#include <vector>
#include <memory>

#include "glGame.h"

#include "parser.h"

const double maxEnergy = 100;

class World;
class Resource : public RectPositional
{
protected:
    double energy = 1;
    bool dead = false;
public:
    Resource(const glm::vec4& pos_,double energy_);
    virtual void update(World& world);
    void setPos(const glm::vec2& pos);
    double getEnergy();
    void setEnergy(double amount);
    virtual void collide(Resource& other);
    void setDead(bool dead_);
    bool getDead();

};

class Food : public Resource
{
public:
    Food(const glm::vec2& pos);
};

class Creature : public Resource
{
    Program prog;
    DNA genes;
    DNA mutate();
public:
    Creature(const glm::vec2& pos_, const DNA& genes_);
    Creature(const glm::vec2& pos_, std::string filename);
    void update(World& world);
    void collide(Resource& other);
};

class World
{
    std::unique_ptr<RawQuadTree> tree;
    int ticks = 0;
public:
    std::vector<std::unique_ptr<Resource>> resources;

    World();
    void update();
    void addResource(Resource& resource);
    void removeResource(Resource& resource);
    void adjustPos(Resource& resource);
    RawQuadTree* getTree();
};



#endif // CREATURES_H_INCLUDED

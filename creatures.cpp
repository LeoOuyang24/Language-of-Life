#define _USE_MATH_DEFINES
#include <math.h>

#include "render.h"
#include "SDLhelper.h"

#include "creatures.h"

Resource::Resource(const glm::vec4& pos_,double energy_) : RectPositional(pos_), energy(energy_)
{

}

void Resource::update(World& world)
{

}

void Resource::setPos(const glm::vec2& pos)
{
    rect.x = pos.x;
    rect.y = pos.y;
}

void Resource::collide(Resource& other)
{

}

double Resource::getEnergy()
{
    return energy;
}


void Resource::setEnergy(double amount)
{
    energy = std::max(std::min(maxEnergy, energy + amount),0.0);
}

void Resource::setDead(bool dead_)
{
    dead = dead_;
}

bool Resource::getDead()
{
    return dead || energy <= 0;
}

Food::Food(const glm::vec2& pos) : Resource({pos.x,pos.y,10,10},5)
{

}

Creature::Creature(const glm::vec2& pos_,const DNA& genes_) : Resource({pos_.x,pos_.y,20,20},70),  genes(genes_)
{
    prog.setDNA(genes);
}

Creature::Creature(const glm::vec2& pos_, std::string filename) : Resource({pos_.x, pos_.y, 20, 20}, 70), genes(parse(filename))
{
    prog.setDNA(genes);
}

DNA Creature::mutate()
{
    DNA mutant;
    int size = genes.size();
    std::cout << "DNA:\n";
    for (int i = 0; i < size; ++i)
    {
        std::string gene = "";
        int length = genes[i].size();
        for (int j = 0; j < length; ++j)
        {
            if (rand()%10 == 0)
            {
                int defect = rand()%5;
                switch (defect)
                    {
                        case 0: length --; break;
                        case 1: length ++; break;
                        case 2: gene += ' '; gene += rand()%96 + 32; gene += ' ';  break;
                        case 3: gene += rand()%96 + 32;
                        default: gene += rand()%2 + 40; gene += " = "; break;
                    }
            }
            else
            {
                if (j >= genes[i].size())
                {
                    gene += ' ';
                }
                else
                {
                    gene += genes[i][j];
                }
            }
        }
        mutant.push_back(gene);
        if (rand()% 10 == 0)
        {
            mutant.push_back(gene);
        }
        std::cout << gene << "\n";
    }
    std::cout << "END\n";
    return mutant;
}

void Creature::update(World& world)
{
    prog.step();
    if (prog.isFinished())
    {
        glm::vec2 newPos = prog.getResult();
        rect.x += newPos.x;
        rect.y += newPos.y;
        //std::cout << newPos.x << " " << newPos.y << std::endl;
        setEnergy(-.01);
        prog.reset();
    }


    std::vector<Positional* > nearby;
     world.getTree()->getNearest(nearby,*this);
    for (int i = nearby.size() - 1; i >= 0; i-- )
    {
        Resource* other = static_cast<Resource*>(nearby[i]);
        if (other != this && vecIntersect(rect,other->getRect()))
        {
            collide(*other);
        }
       //PolyRender::requestRect(other->getRect(),{1,0,0,1},true,0,1);
    }

    if  (energy > .7*maxEnergy)
    {
        setEnergy(-.7*maxEnergy);
        double theta = (rand()%360)*M_PI/180;
        world.addResource(*(new Creature({rect.x + rect.z/2 + 2*rect.z*cos(theta), rect.y + rect.a/2 + 2*rect.a*sin(theta)},mutate())));
    }
}

void Creature::collide(Resource& other)
{
   // if (other.getEnergy() < energy)
    {
        setEnergy(other.getEnergy());
        other.setDead(true);
    }
}

World::World()
{
    glm::vec2 screenDimen = RenderProgram::getScreenDimen();
    tree.reset(new RawQuadTree({0,0,screenDimen.x,screenDimen.y}));
    for (int i = 0; i < 100; i ++)
    {
        addResource(*new Food({rand()%((int)screenDimen.x),rand()%((int)screenDimen.y)}));
    }
    for (int i = 0; i < 1; ++i)
    {
        addResource(*(new Creature({320 + (i - 5)*30,320},"programs/scan.txt")));
    }
}

void World::update()
{
    const glm::vec4* treeRect = &(tree->getRect());
    auto mousePos = MouseManager::getMousePos();
    for (int i = resources.size() - 1 ; i >= 0; i--)
    {
        Resource* current = resources[i].get();
        if (current->getDead())
        {
            removeResource(*current);
        }
        else
        {
            RawQuadTree* oldTree = tree->find(*current);
            current->update(*this);
            adjustPos(*current);
            tree->update(*current, *oldTree);

            if (MouseManager::getJustClicked() == SDL_BUTTON_LEFT && pointInVec(current->getRect(),mousePos.first,mousePos.second,0))
            {
                RawQuadTree* newTree = tree->find(*current);
                //std::cout << newTree->getRect().x << " " << newTree->getRect().y << " " << newTree->getRect().z << std::endl;
            }
            PolyRender::requestRect(resources[i]->getRect(), {0,resources[i]->getEnergy()/maxEnergy,resources[i]->getEnergy()/maxEnergy,1},true,0,0);
        }
    }
    ticks ++;
    if (ticks % 10 == 0)
    {
        int random = rand()%10 - 8;
        for (int i = 0; i < random; ++i)
        {
            addResource(*new Food({rand()%((int)treeRect->z),rand()%((int)treeRect->a)}));
        }
    }
    //tree->render({0,0});

}

void World::addResource(Resource& resource)
{
    adjustPos(resource);
    tree->add(resource);
    resources.emplace_back(&resource);
}

void World::removeResource(Resource& resource)
{
    tree->remove(resource);
    int size = resources.size();
    for(int i = 0; i < size; ++i)
    {
        if (resources[i].get() == &resource)
        {
            resources.erase(resources.begin() + i);
            break;
        }
    }
}

void World::adjustPos(Resource& resource)
{
    glm::vec2 moveTo = resource.getPos();
    const glm::vec4* rect = &resource.getRect();
    const glm::vec4* treeRect = &(tree->getRect());
    if (rect->x > treeRect->x + treeRect->z)
    {
       moveTo.x = treeRect->x;
    }
    else if (rect->x + rect->z < treeRect->x)
    {
        moveTo.x = treeRect->x + treeRect->z;
    }
    if (rect->y > treeRect->y + treeRect->a)
    {
        moveTo.y = treeRect->y;
    }
    else if (rect->y + rect->a < treeRect->y)
    {
        moveTo.y = treeRect->y + treeRect->a;
    }
    resource.setPos(moveTo);

}

RawQuadTree* World::getTree()
{
    return tree.get();
}


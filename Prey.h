#include "Org.h"


/**
 * Creates a Prey organism, which is initialized with fewer strength points (and thus smaller mutation amounts) and quicker reproduction than other organisms.
 */
class Prey : public Organism {

    public:
        Prey(emp::Ptr<emp::Random> _random, double _points=0.0, double _strength = 100.0) : Organism(_random, _points,  _strength){;}
        
        std::string GetType() override {return "Prey";}
        

        /**
         * If a Prey organism can reproduce, creates its offspring.
         * @return Pointer to offspring (null pointer if the Prey organism cannot reproduce)
         */
        emp::Ptr<Organism> CheckReproduction() override{
            if (GetPoints()>= 500.0) {
                Prey* offspring = new Prey(*this);
                offspring->mutate(*GetRandom());
                offspring->SetPoints(0.0);
                AddPoints(-500.0);
                return offspring;
            }
            else {
                return nullptr;
            }
        }
        
        /**
         * Mutates a Prey organism's strength value by a small, pseudorandom amount
         * @param random Pseudorandom number generator
         */
        virtual void mutate(emp::Random random) override {
            double randomNum = random.GetRandNormal(0.0, 1.0);
            AddStrength(randomNum);
        }
};
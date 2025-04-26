#include "Org.h"


/**
 * Creates a Predator organism, which is initialized with more strength points (and thus larger mutation amounts) than other organisms.
 */
class Predator : public Organism {

    public:
        Predator(emp::Ptr<emp::Random> _random, double _points=0.0, double _strength = 2000.0) : Organism(_random, _points,  _strength){;}
        
        std::string GetType() override {return "Predator";}
        
        /**
         * If a Predator can reproduce, creates its offspring.
         * @return Pointer to offspring (null pointer if the Predator cannot reproduce)
         */
        emp::Ptr<Organism> CheckReproduction() override{
            if (GetPoints()>= 1000.0) {
                Predator* offspring = new Predator(*this);
                offspring->mutate(*GetRandom());
                offspring->SetPoints(0.0);
                AddPoints(-1000.0);
                return offspring;
            }
            else {
                return nullptr;
            }
        }

        /**
         * Mutates a Predator's strength value by a small, pseudorandom amount
         * @param random Pseudorandom number generator
         */
        virtual void mutate(emp::Random random) override {
            double randomNum = random.GetRandNormal(0.0, 20.0);
            AddStrength(randomNum);
        }
};
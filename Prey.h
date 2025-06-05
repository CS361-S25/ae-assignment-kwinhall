#ifndef PREY_H
#define PREY_H

#include "Org.h"
#include "Predator.h"


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


        /**
         * Facilitates a competition for space between two organisms, updating organisms' strength levels and death statuses accordingly.
         * @param org2 The second organism in the interaction.
         * @param org1Position The first organism's position in the population.
         * @param org2Position The second organism's position in the population.
         * @return position of the organism that will die.
         */
        virtual int Interact(Organism* org2, int org1Position, int org2Position) override {
            int indexToDie;
            if (org2->GetType() == "Predator") { // if the other org is a predator, it hunts the prey
                indexToDie = dynamic_cast<Predator*>(org2)->Hunt(this, org2Position, org1Position);
            }
            else { // otherwise, the prey fights the other org to the death
                indexToDie = Fight(org2, org1Position, org2Position);
            }
            return indexToDie;
        }
};
#endif
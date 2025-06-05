#ifndef PREDATOR_H
#define PREDATOR_H

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


        /**
         * Facilitates a hunt, updating predator's and prey's strength levels and death status accordingly.
         * @param prey The organism that is being hunted.
         * @param predatorPosition The predator's position in the population.
         * @param preyPosition The prey's position in the population.
         * @return position of the organism that will die.
         */
        int Hunt(Organism* prey, int predatorPosition, int preyPosition) {
            double predatorStrength = GetStrength();
            double preyStrength = prey->GetStrength();
            // if predator is stronger, it kills and eats the prey, gaining its strength points
            if (predatorStrength >= preyStrength) {
                AddStrength(preyStrength);
                return preyPosition;
            }
            // if the prey is stronger, it kills the predator
            else {
                prey->AddStrength(0.05 * predatorStrength);
                return predatorPosition;
            }
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
            if (org2->GetType() == "Prey") { // if the other org is prey, the predator hunts it
                indexToDie = Hunt(org2, org1Position, org2Position);
            }
            else { // otherwise, predator fights the other org to the death
                indexToDie = Fight(org2, org1Position, org2Position);
            }
            return indexToDie;
        }

};
#endif
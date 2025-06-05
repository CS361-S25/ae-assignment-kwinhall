#ifndef ORG_H
#define ORG_H

#include "emp/math/Random.hpp"
#include "emp/tools/string_utils.hpp"
#include <string>


/**
 * Creates an Organism with the ability to reproduce, mutate, and vary reproduction points and strength.
 */
class Organism {
    private:
        double points; // points for reproductive ability/maturity
        emp::Ptr<emp::Random> random;
        double strength; // points for strength (hunting, fighting, and overall survival)

    public:
        Organism(emp::Ptr<emp::Random> _random, double _points = 1000.0, double _strength = 500.0) :
            points(_points), random(_random), strength(_strength){;}

        void SetPoints(double _in) {points = _in;}
        double GetPoints() {return points;}
        void AddPoints(double _in) {points += _in;}

        double GetStrength() {return strength;}
        void AddStrength(double _in) {strength += _in;}

        emp::Ptr<emp::Random> GetRandom() {return random;}

        virtual std::string GetType() {return "Org";}


        /**
         * If an Organism can reproduce, creates its offspring.
         * @return Pointer to offspring (null pointer if the Organism cannot reproduce)
         */
        virtual emp::Ptr<Organism> CheckReproduction() {
            if (points >= 1000) { // Organism has sufficient reproduction points
                emp::Ptr<Organism> offspring = new Organism(*this);
                offspring->mutate(*random);
                offspring->points = 0.0;
                points -= 1000;
                return offspring;
            }
            else { // Organism cannot reproduce yet
                return nullptr;
            }
        }

        
        /**
         * Mutates an Organism's strength value by a small, pseudorandom amount
         * @param random Pseudorandom number generator
         */
        virtual void mutate(emp::Random random) {
            double randomNum = random.GetRandNormal(0.0, 5.0);
            strength += randomNum;
        }


        /**
         * Facilitates a fight between two organisms, updating organisms' strength levels and death statuses accordingly.
         * @param org2 The second organism in the fight.
         * @param org1Position The first organism's position in the population.
         * @param org2Position The second organism's position in the population.
         * @return position of the organism that will die.
         */
        int Fight(Organism* org2, int org1Position, int org2Position) {
            // the stronger organism wins the fight
            if (GetStrength() >= org2->GetStrength()) {
                AddStrength(0.05 * org2->GetStrength());
                return org2Position;
            }
            else {
                org2->AddStrength(0.05 * GetStrength());
                return org1Position;
            }
        }


        /**
         * Facilitates a competition for space between two organisms, updating organisms' strength levels and death statuses accordingly.
         * @param org2 The second organism in the interaction.
         * @param org1Position The first organism's position in the population.
         * @param org2Position The second organism's position in the population.
         * @return position of the organism that will die.
         */
        virtual int Interact(Organism* org2, int org1Position, int org2Position) {
            int indexToDie = Fight(org2, org1Position, org2Position);
            return indexToDie;
        }
        
};
#endif
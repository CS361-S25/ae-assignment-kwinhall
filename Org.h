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
};
#endif
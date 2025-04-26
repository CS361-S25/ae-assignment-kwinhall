#ifndef WORLD_H
#define WORLD_H

#include "emp/Evolve/World.hpp"
#include "emp/math/random_utils.hpp"
#include "emp/math/Random.hpp"
#include <math.h>
#include <algorithm>
#include "Org.h"


/**
 * Creates a World populated by Organisms that evolve and engage in ecological interactions (predation and competition).
 */
class OrgWorld : public emp::World<Organism> {
    emp::Random &random;
    emp::Ptr<emp::Random> random_ptr;


    public:

    OrgWorld(emp::Random &_random) : emp::World<Organism>(_random), random(_random) {
        random_ptr.New(_random);
    }

    ~OrgWorld() {
    }


    /**
     * Removes the specified organism from the population and returns it. 
     * @param i The organism's position in the population.
     * @return The organism that was removed.
     */
    emp::Ptr<Organism> ExtractOrganism(int i) {
        emp::Ptr<Organism> extractedOrganism = pop[i];
        pop[i] = nullptr;
        return extractedOrganism;
    }
    

    /**
     * Moves an organism to a random nearby position, resolving spatial conflicts through competition and predation. 
     * @param currentIndex The organism's current position in the population.
     * @return The organism's new position in the population (or -infinity if the organism died).
     */
    int MoveOrganism(int currentIndex) {
        int movedIndex;
        emp::WorldPosition positionToMove = GetRandomNeighborPos(currentIndex);
        emp::Ptr<Organism> organismToMove = ExtractOrganism(currentIndex);
        int indexToMove = positionToMove.GetIndex();

        if (IsOccupied(positionToMove)) { // two organisms will fight over who occupies the position
            emp::Ptr<Organism> winner;
            emp::Ptr<Organism> existingOrganism = ExtractOrganism(indexToMove);
            if (organismToMove->GetType() == existingOrganism->GetType()){
                Fight(organismToMove, existingOrganism, currentIndex, indexToMove);
            }
            else if (organismToMove->GetType() == "Predator" and existingOrganism->GetType() == "Prey") {
                Hunt(organismToMove, existingOrganism, currentIndex, indexToMove);
            }
            else if (existingOrganism->GetType() == "Predator" and organismToMove->GetType() == "Prey"){
                Hunt(existingOrganism, organismToMove, indexToMove, currentIndex);
            }
            else {/*at least one organism is not predator or prey--do nothing*/}
        }

        if (organismToMove == nullptr) { // organism died in fight/hunt
            return -INFINITY;
        }
        else if (!IsOccupied(positionToMove)) { // space was empty to begin with or organism won fight/hunt
            AddOrgAt(organismToMove, positionToMove);
            return indexToMove;
        }
        else { // organism is alive but did not move -- only occurs if at least one organism involved was not a predator or prey
            return currentIndex;
        }
    }


    /**
     * Executes a hunt, updating predator's and prey's strength values and death status accordingly.
     * @param predator The organism that is hunting.
     * @param prey The organism that is being hunted.
     * @param predatorPosition The predator's position in the population.
     * @param preyPosition The prey's position in the population.
     */
    void Hunt(emp::Ptr<Organism> predator, emp::Ptr<Organism> prey, int predatorPosition, int preyPosition) {
        if (predator != nullptr and prey != nullptr) {
            double predatorStrength = predator->GetStrength();
            double preyStrength = prey->GetStrength();

            // if predator is stronger, it kills and eats the prey, gaining its strength points
            if (predatorStrength >= preyStrength) {
                predator->AddStrength(preyStrength);
                DoDeath(preyPosition);
            }
            // if the prey is stronger, the predator dies instead
            else {
                DoDeath(predatorPosition);
            }
        }
    }


    /**
     * Executes a competition for space between two organisms, updating organisms' strength values and death status accordingly.
     * @param org1 The first organism in the fight.
     * @param org2 The second organism in the fight.
     * @param org1Position The first organism's position in the population.
     * @param org2Position The second organism's position in the population.
     */
    void Fight(emp::Ptr<Organism> org1, emp::Ptr<Organism> org2, int org1Position, int org2Position) {
        if (org1 != nullptr and org2 != nullptr) {
            emp::Ptr<Organism> winner;
            emp::Ptr<Organism> loser;
            int loserLocation;
    
            if (org1->GetStrength() >= org2->GetStrength()) {
                winner = org1;
                loser = org2;
                loserLocation = org2Position;
            }
            else {
                winner = org2;
                loser = org1;
                loserLocation = org1Position;
            }
            // winner kills opponent and gains some of the opponent's strength
            winner->AddStrength(0.05 * loser->GetStrength());
            DoDeath(loserLocation);
        }
    }


    void DoPredation(int idx) {
        int numHunts = 0;
        emp::Ptr<Organism> currentOrganism = pop[idx];
        for (int i=0;i<4;i++){
            int randomIndex = GetRandomNeighborPos(idx).GetIndex();
            if (IsOccupied(randomIndex)) {
                emp::Ptr<Organism> neighborOrganism = pop[randomIndex];
                if (neighborOrganism->GetType() == "Prey") {
                    Hunt(currentOrganism, neighborOrganism, idx, randomIndex);
                    numHunts++;
                }
            }
        }
        currentOrganism = pop[idx];
        if (numHunts == 0 and currentOrganism != nullptr){
            currentOrganism->AddStrength(-.01 * currentOrganism->GetStrength());
        }
    }


    void ManageStrength(int idx) {
        emp::Ptr<Organism> currentOrganism = pop[idx];
        if (currentOrganism != nullptr) {
            if (currentOrganism->GetType() == "Prey" and currentOrganism->GetStrength() < 50.0) {
                DoDeath(idx);
            }
            else if (currentOrganism->GetType() == "Predator" and currentOrganism->GetStrength() < 50.0) {
                DoDeath(idx);
            }
        }
    }


    void ManageReproduction(int idx) {
        emp::Ptr<Organism> currentOrganism = pop[idx];
        if (currentOrganism != nullptr) {
            emp::Ptr<Organism> offspring = currentOrganism->CheckReproduction(); 
            if(offspring) { // give birth to offspring
                AddOrgAt(offspring, GetRandomNeighborPos(idx));
            }
        }
    }


    void Update() {
        emp::World<Organism>::Update();
        emp::vector<size_t> schedule = emp::GetPermutation(random, GetSize());
        for (int i : schedule) {
            if(!IsOccupied(i)) {
                continue;
            }
            else {
                pop[i]->AddPoints(100.0);
            }
        }

        schedule = emp::GetPermutation(random, GetSize());
        for (int i : schedule) {
            if(!IsOccupied(i)) {
                continue;
            }
            else {
                emp::Ptr<Organism> currentOrganism = pop[i];
                int newIndex = MoveOrganism(i);
                if (currentOrganism->GetType() == "Predator"){
                    DoPredation(newIndex);
                }
                ManageStrength(newIndex);
                ManageReproduction(newIndex);
            }
        } 
    }

};
#endif
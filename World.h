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
        emp::Ptr<Organism> organismToMove = ExtractOrganism(currentIndex);
        emp::WorldPosition positionToMove = GetRandomNeighborPos(currentIndex);
        int indexToMove = positionToMove.GetIndex();

        // if the position is already occupied, the organisms will compete over who will occupy the position next
        if (IsOccupied(positionToMove)) {
            emp::Ptr<Organism> winner;
            emp::Ptr<Organism> existingOrganism = ExtractOrganism(indexToMove);

            // if the organisms are the same type, they fight
            if (organismToMove->GetType() == existingOrganism->GetType()){
                Fight(organismToMove, existingOrganism, currentIndex, indexToMove);
            }
            // otherwise, the predator hunts the prey
            else if (organismToMove->GetType() == "Predator" and existingOrganism->GetType() == "Prey") {
                Hunt(organismToMove, existingOrganism, currentIndex, indexToMove);
            }
            else if (existingOrganism->GetType() == "Predator" and organismToMove->GetType() == "Prey"){
                Hunt(existingOrganism, organismToMove, indexToMove, currentIndex);
            }
        }

        if (organismToMove == nullptr) { // organism died in the fight/hunt, return an invalid position
            return -INFINITY;
        }
        else if (!IsOccupied(positionToMove)) { // space was empty to begin with or organism won fight/hunt, return new position
            AddOrgAt(organismToMove, positionToMove);
            return indexToMove;
        }
        else { // organism is alive but did not move. this could only occur for organisms that are neither predator nor prey
            return currentIndex;
        }
    }


    /**
     * Facilitates a hunt, updating predator's and prey's strength levels and death status accordingly.
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
            // if the prey is stronger, it kills the predator
            else {
                prey->AddStrength(0.05 * predatorStrength);
                DoDeath(predatorPosition);
            }
        }
    }


    /**
     * Facilitates a competition for space between two organisms, updating organisms' strength levels and death statuses accordingly.
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
    
            // the stronger organism wins the fight
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


    /**
     * Makes predator hunt in nearby area for sustenance, impacting its strength level.
     * @param predatorPosition The predator's current position in the population.
     */
    void DoPredation(int predatorPosition) {
        int numKills = 0;
        emp::Ptr<Organism> predator = pop[predatorPosition];

        // predator checks 4 random nearby locations and hunts if prey is present
        for (int i=0;i<4;i++){
            int randomIndex = GetRandomNeighborPos(predatorPosition).GetIndex();
            if (IsOccupied(randomIndex)) {
                emp::Ptr<Organism> neighbor = pop[randomIndex];
                
                if (neighbor->GetType() == "Prey") {
                    Hunt(predator, neighbor, predatorPosition, randomIndex);
                    if (predator != nullptr) { // if predator is alive, it successfully killed its prey
                        numKills++;
                    }
                }
            }
        }
        // if predator is still alive but it hasn't killed any prey, reduce its strength due to lack of food
        predator = pop[predatorPosition];
        if (predator != nullptr and numKills == 0){
            predator->AddStrength(-.01 * predator->GetStrength());
        }
    }
    

    /**
     * Checks an organism's strength levels, facilitating its death if they are very low.
     * @param currentIndex The organism's current position in the population.
     */
    void ManageStrength(int currentIndex) {
        emp::Ptr<Organism> currentOrganism = pop[currentIndex];
        if (currentOrganism != nullptr) {
            if (currentOrganism->GetStrength() < 50.0) {
                DoDeath(currentIndex);
            }
        }
    }


    /**
     * Checks an organism's ability to reproduce, placing offspring in a random location nearby if successful.
     * @param currentIndex The organism's current position in the population.
     */
    void ManageReproduction(int currentIndex) {
        emp::Ptr<Organism> currentOrganism = pop[currentIndex];
        if (currentOrganism != nullptr) {
            emp::Ptr<Organism> offspring = currentOrganism->CheckReproduction(); 
            if(offspring) { // give birth to offspring
                AddOrgAt(offspring, GetRandomNeighborPos(currentIndex));
            }
        }
    }


    /**
     * Updates reproduction points and strength levels for each organism and faciliates movement, hunting, death, and reproduction.
     */
    void Update() {
        emp::World<Organism>::Update();
        // permutation ensures that organisms early in the population don't have an advantage
        emp::vector<size_t> schedule = emp::GetPermutation(random, GetSize()); 
        for (int i : schedule) {
            if(!IsOccupied(i)) {
                continue;
            }
            else {
                pop[i]->AddPoints(100.0); // as time passes, organism's ability to reproduce increases
            }
        }

        // permutation ensures that organisms early in the population don't have an advantage
        schedule = emp::GetPermutation(random, GetSize());
        for (int i : schedule) {
            if(!IsOccupied(i)) {
                continue;
            }
            else {
                // facilitates movement, hunting, death, and reproduction for each organism
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
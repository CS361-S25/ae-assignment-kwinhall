#ifndef WORLD_H
#define WORLD_H

#include "emp/Evolve/World.hpp"
#include "emp/math/random_utils.hpp"
#include "emp/math/Random.hpp"
#include <math.h>
#include <algorithm>
#include "Org.h"
#include "Predator.h"
#include "Prey.h"


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
     * Gets the specified predator from the population and returns it. 
     * @param i The predator's position in the population.
     * @return A pointer to the predator.
     */
    Predator* GetPredator(int i) {
        Organism* org = pop[i];
        if (org == nullptr) {
            return nullptr;
        }
        else if (org->GetType() == "Predator") {
            Predator* predator = dynamic_cast<Predator*>(org);
            return predator;
        }
        return nullptr;
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

        // if the position is already occupied, the organisms compete to the death over who will occupy the position next
        if (IsOccupied(positionToMove)) {
            emp::Ptr<Organism> existingOrganism = ExtractOrganism(indexToMove);
            int indexToDie = organismToMove->Interact(existingOrganism, currentIndex, indexToMove);
            DoDeath(indexToDie); // loser of interaction dies
        }

        // if the organism died in the interaction, return an invalid position
        if (organismToMove == nullptr) {
            return -INFINITY;
        }
        // if the space was empty to begin with or organism won the interaction, return its new position
        else if (!IsOccupied(positionToMove)) {
            AddOrgAt(organismToMove, positionToMove);
            return indexToMove;
        }
        else {
            return currentIndex;
        }
    }


    /**
     * Makes the predator at the given location hunt for sustenance, impacting organisms in nearby locations.
     * @param predatorPosition The predator's current position in the population.
     */
    void ManageHuntingAtIdx(int predatorPosition) {
        int numKills = 0;
        Predator* predator = GetPredator(predatorPosition);

        for (int i=0;i<4;i++){ // predator checks 4 random nearby locations and hunts if prey is present
            int randomIndex = GetRandomNeighborPos(predatorPosition).GetIndex();
            if (IsOccupied(randomIndex) and predator != nullptr) {

                emp::Ptr<Organism> neighbor = pop[randomIndex];
                if (neighbor->GetType() == "Prey") {
                    predator = GetPredator(predatorPosition);
                    int indexToDie = predator->Hunt(neighbor, predatorPosition, randomIndex);
                    DoDeath(indexToDie);

                    // if predator is still alive, it successfully killed its prey
                    predator = GetPredator(predatorPosition);
                    if (predator != nullptr) { numKills++; }
                    else { break; }
                }
            }
        }

        // if predator is still alive but hasn't eaten any prey, reduce its strength due to lack of food
        predator = GetPredator(predatorPosition);
        if (predator != nullptr and numKills == 0){
            predator->AddStrength(-.01 * predator->GetStrength());
        }
    }
    

    /**
     * Checks the strength levels of the organism at the given location and facilitates its death if strength levels are low.
     * @param currentIndex The organism's current position in the population.
     */
    void ManageDeathAtIdx(int currentIndex) {
        emp::Ptr<Organism> currentOrganism = pop[currentIndex];
        if (currentOrganism != nullptr) {
            if (currentOrganism->GetStrength() < 50.0) {
                DoDeath(currentIndex);
            }
        }
    }


    /**
     * Checks the reproductive ability of the organism at the given location, placing offspring in a random location nearby if successful.
     * @param currentIndex The organism's current position in the population.
     */
    void ManageReproductionAtIdx(int currentIndex) {
        emp::Ptr<Organism> currentOrganism = pop[currentIndex];
        if (currentOrganism != nullptr) {
            emp::Ptr<Organism> offspring = currentOrganism->CheckReproduction(); 
            if(offspring) { // give birth to offspring and add it to world
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
                // moves organism and executes changes (hunting, death, and reproduction) at its new position
                emp::Ptr<Organism> currentOrganism = pop[i];
                int newIndex = MoveOrganism(i);
                if (newIndex != -INFINITY) {
                    if (currentOrganism->GetType() == "Predator"){
                        ManageHuntingAtIdx(newIndex); // executes hunt in and around newIndex
                    }
                    ManageDeathAtIdx(newIndex); // determines if org at newIndex should die
                    ManageReproductionAtIdx(newIndex); // determines if org at newIndex should reproduce
                }
            }
        } 
    }

};
#endif
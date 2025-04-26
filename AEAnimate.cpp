/**
 * Kendra Winhall
 * Artificial Ecologies Assignment, CS 361
 * Adapted from starter code from Anya Vostinar
 * 
 * Implements an artificial ecosystem with predators and prey on a toroidal grid.
 * Run with ./compile-run.sh and view web interface at http://localhost:8000/ (Chrome or Safari recommended).
 */


#include "emp/math/Random.hpp"
#include "emp/web/Animate.hpp"
#include "emp/web/web.hpp"
#include "World.h"
#include "Org.h"
#include "Predator.h"
#include "Prey.h"

emp::web::Document doc{"target"};

/**
 * Manages the animation of an artificial ecosystem with predators and prey.
 */
class AEAnimate : public emp::web::Animate {

    // arena width and height
    const int num_h_boxes = 70;
    const int num_w_boxes = 70;
    const double RECT_SIDE = 8; // grid cell length in pixels
    const double width{num_w_boxes * RECT_SIDE};
    const double height{num_h_boxes * RECT_SIDE};

    emp::Random random{2};
    OrgWorld world{random};

    emp::web::Canvas canvas{width, height, "canvas"};

    public:

    /**
    * Creates web interface and initializes population.
    */
    AEAnimate() {

        doc << "<div style \"display: flex\">";
        doc << "<div>" << canvas.SetCSS("margin", "8px").SetCSS("float", "left") << GetToggleButton("Toggle") << GetStepButton("Step") << "</div>";
        doc << "<div><p>" << "This program implements an artificial ecosystem with two species that evolve and engage in ecological interactions (predation and competition) on a toroidal grid. Predators are depicted as red, prey are depicted as blue, and uninhabited squares of grass are depicted as green." << "</p>";
        doc << "<p>" << "Predators are stronger and reproduce slower, while prey are weaker but reproduce faster. Predators hunt prey and eat them for strength (predation). If they're strong enough, prey fend off the attacks and kill the predator that attacks them. Organisms also compete for resources within their own species (competition). In particular, when two organisms wish to inhabit the same square of grass, they fight to the death to settle the dispute." << "</p></div>";

        world.Resize(num_w_boxes, num_h_boxes);  
        world.SetPopStruct_Grid(num_w_boxes, num_h_boxes);

        // add predators and prey to random locations in the population
        for (int i = 0; i < 10; i++) {
            Predator* newPredator = new Predator(&random);
            newPredator->mutate(random);
            world.AddOrgAt(newPredator, random.GetUInt((num_w_boxes * num_h_boxes)-1));
        }
        for (int i = 0; i < 20; i++) {
            Prey* newPrey = new Prey(&random);
            newPrey->mutate(random);
            world.AddOrgAt(newPrey, random.GetUInt((num_w_boxes * num_h_boxes)-1));
        }
    }

    /**
     * Updates the ecosystem animation for each frame.
     */
    void DoFrame() override {
        canvas.Clear(); 
        world.Update();

        int org_num = 0;
        std::string color;

        // iterates through population and color codes organisms by type
        for (int x = 0; x < num_w_boxes; x++){
            for (int y = 0; y < num_h_boxes; y++) {
                if (world.IsOccupied(org_num)) {
                    if (world.GetOrg(org_num).GetType() == "Predator") {color = "red";}
                    else if (world.GetOrg(org_num).GetType() == "Prey") {color = "blue";}
                    else {color = "black";}
                } 
                else { // empty cells are "grass"
                    color = "green"; 
                }
                canvas.Rect(x * RECT_SIDE, y * RECT_SIDE, RECT_SIDE, RECT_SIDE, color, color); // draws organisms
                org_num++;
            }
        }
    }
};

AEAnimate animate;

int main() {animate.Step();}
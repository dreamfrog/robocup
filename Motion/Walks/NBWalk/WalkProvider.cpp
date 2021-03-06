
// This file is part of Man, a robotic perception, locomotion, and
// team strategy application created by the Northern Bites RoboCup
// team of Bowdoin College in Brunswick, Maine, for the Aldebaran
// Nao robot.
//
// Man is free software: you can redistribute it and/or modify
// it under the terms of the GNU Lesser Public License as published by
// the Free Software Foundation, either version 3 of the License, or
// (at your option) any later version.
//
// Man is distributed in the hope that it will be useful,
// but WITHOUT ANY WARRANTY; without even the implied warranty of
// MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
// GNU Lesser Public License for more details.
//
// You should have received a copy of the GNU General Public License
// and the GNU Lesser Public License along with Man.  If not, see
// <http://www.gnu.org/licenses/>.

#include <boost/shared_ptr.hpp>
using boost::shared_ptr;

#include "NBInclude/Sensors.h"
#include "WalkProvider.h"
using namespace std;

using namespace Kinematics;

#include "debug.h"

WalkProvider::WalkProvider(shared_ptr<Sensors> s)
    : MotionProvider(WALK_PROVIDER),
      sensors(s),
      metaGait(),
      nextGait(DEFAULT_GAIT),
      startGait(DEFAULT_GAIT),
      stepGenerator(sensors,&metaGait),
      pendingCommands(false),
      pendingStepCommands(false),
      pendingGaitCommands(false),
      pendingStartGaitCommands(false),
      nextCommand(NULL)
{
    pthread_mutex_init(&walk_provider_mutex,NULL);

    setActive();
}

WalkProvider::~WalkProvider() {
    pthread_mutex_destroy(&walk_provider_mutex);
}

void WalkProvider::requestStopFirstInstance() {
    setCommand(new WalkCommand(0.0f, 0.0f, 0.0f));
}

void WalkProvider::hardReset(){
    pthread_mutex_lock(&walk_provider_mutex);
    stepGenerator.resetHard();
    pendingCommands = pendingStepCommands =false;
    setActive();
    pthread_mutex_unlock(&walk_provider_mutex);
}

void WalkProvider::calculateNextJointsAndStiffnesses() 
{
#if DEBUG_NUMOTION_VERBOSITY > 4
    debug << "WalkProvider::calculateNextJointsAndStiffnesses()"<<endl;
#endif
    pthread_mutex_lock(&walk_provider_mutex);
    if ( pendingGaitCommands){
        if(stepGenerator.isDone() && pendingStartGaitCommands){
            metaGait.setStartGait(startGait);
        }
        metaGait.setNewGaitTarget(nextGait);
    }
    pendingGaitCommands = false;
    pendingStartGaitCommands = false;

    //The meta gait needs to be ticked BEFORE any commands are sent to stepGen
    metaGait.tick_gait();

    if(nextCommand)
    {
        stepGenerator.setSpeed(nextCommand->x_mms,
                               nextCommand->y_mms,
                               nextCommand->theta_rads);
    }
    pendingCommands = false;
    nextCommand = NULL;

    if(pendingStepCommands){
        stepGenerator.takeSteps(nextStepCommand->x_mms,
                                nextStepCommand->y_mms,
                                nextStepCommand->theta_rads,
                                nextStepCommand->numSteps);
    }
    pendingStepCommands=false;

    //Also need to process stepCommands here

    if(!isActive()){
        cout << "WARNING, I wouldn't be calling the Walkprovider while"
            " it thinks its DONE if I were you!" <<endl;
    }

    //ask the step Generator to update ZMP values, com targets
    stepGenerator.tick_controller();

    // Now ask the step generator to get the leg angles
    WalkLegsTuple legs_result = stepGenerator.tick_legs();

    //Finally, ask the step generator for the arm angles
    WalkArmsTuple arms_result = stepGenerator.tick_arms();

    //Get the joints and stiffnesses for each Leg
    vector<float> lleg_joints = legs_result.get<LEFT_FOOT>().get<JOINT_INDEX>();
    vector<float> rleg_joints = legs_result.get<RIGHT_FOOT>().get<JOINT_INDEX>();
    vector<float> lleg_gains = legs_result.get<LEFT_FOOT>().get<STIFF_INDEX>();
    vector<float> rleg_gains = legs_result.get<RIGHT_FOOT>().get<STIFF_INDEX>();

    //grab the stiffnesses for the arms
    vector<float> larm_joints = arms_result.get<LEFT_FOOT>().get<JOINT_INDEX>();
    vector<float> rarm_joints = arms_result.get<RIGHT_FOOT>().get<JOINT_INDEX>();
    vector<float> larm_gains = arms_result.get<LEFT_FOOT>().get<STIFF_INDEX>();
    vector<float> rarm_gains = arms_result.get<RIGHT_FOOT>().get<STIFF_INDEX>();


    //Return the joints for the legs
    setNextChainJoints(LARM_CHAIN,larm_joints);
    setNextChainJoints(LLEG_CHAIN,lleg_joints);
    setNextChainJoints(RLEG_CHAIN,rleg_joints);
    setNextChainJoints(RARM_CHAIN,rarm_joints);

    //Return the stiffnesses for each joint
    setNextChainStiffnesses(LARM_CHAIN,larm_gains);
    setNextChainStiffnesses(LLEG_CHAIN,lleg_gains);
    setNextChainStiffnesses(RLEG_CHAIN,rleg_gains);
    setNextChainStiffnesses(RARM_CHAIN,rarm_gains);

    setActive();
    pthread_mutex_unlock(&walk_provider_mutex);
}

void WalkProvider::setCommand(const WalkCommand * command){
    //grab the velocities in mm/second rad/second from WalkCommand
    pthread_mutex_lock(&walk_provider_mutex);
    #if DEBUG_NUMOTION_VERBOSITY > 4
        debug << "WalkProvider::setCommand(" << *command << ") " << command  << "nextCommand: " << nextCommand << endl;
    #endif
    nextCommand = command;
    pendingCommands = true;

    setActive();
    pthread_mutex_unlock(&walk_provider_mutex);
}


void WalkProvider::setCommand(const boost::shared_ptr<Gait> command){
    pthread_mutex_lock(&walk_provider_mutex);
    nextGait = Gait(*command);
    pendingGaitCommands = true;
    pthread_mutex_unlock(&walk_provider_mutex);
}
void WalkProvider::setCommand(const boost::shared_ptr<StepCommand> command){
    pthread_mutex_lock(&walk_provider_mutex);
    nextStepCommand = command;
    pendingStepCommands = true;
    setActive();
    pthread_mutex_unlock(&walk_provider_mutex);
}
void WalkProvider::setActive()
{
    //check to see if the walk engine is active
    if(stepGenerator.isDone() && !pendingCommands && !pendingStepCommands)
    {
        inactive();
    }else{
        active();
    }
}

std::vector<BodyJointCommand *> WalkProvider::getGaitTransitionCommand(){
    pthread_mutex_lock(&walk_provider_mutex);
    vector<float> curJoints = sensors->getMotionBodyAngles();
    vector<float> * gaitJoints = stepGenerator.getDefaultStance(nextGait);

    startGait = nextGait;
    pendingStartGaitCommands = true;

    float max_change = -M_PI_FLOAT*10.0f;
    int max_index = -1;
    for(unsigned int i = 0; i < gaitJoints->size(); i++){

        if (max_change < abs(gaitJoints->at(i) - curJoints.at(i+HEAD_JOINTS)))
            max_index = i;
        max_change = max(max_change,
						 fabs(gaitJoints->at(i) - curJoints.at(i+HEAD_JOINTS)));
    }

    // this is the max we allow, not the max the hardware can do
    const float  MAX_RAD_PER_SEC =  M_PI_FLOAT*0.3f;
    float time = max_change/MAX_RAD_PER_SEC;

    vector<BodyJointCommand *> commands;

    if(time <= MotionConstants::MOTION_FRAME_LENGTH_S)
        return commands;

    //larm: (0.,90.,0.,0.)
    //rarm: (0.,-90.,0.,0.)
    float larm_angles[] = {0.9f, 0.3f,0.0f,0.0f};
    float rarm_angles[] = {0.9f,-0.3f,0.0f,0.0f};

    vector<float> *safe_larm = new vector<float>(larm_angles,
                                                 &larm_angles[ARM_JOINTS]);
    vector<float> * safe_rarm =new vector<float>(rarm_angles,
                                                 &rarm_angles[ARM_JOINTS]);

	// HACK @joho get gait stiffness params. nextGait->maxStiffness
	vector<float> * stiffness = new vector<float>(Kinematics::NUM_JOINTS,
												  0.85f);
	vector<float> * stiffness2 = new vector<float>(Kinematics::NUM_JOINTS,
												  0.85f);

    commands.push_back(new BodyJointCommand(0.5f,safe_larm,NULL,NULL,safe_rarm,
											stiffness,
                                            Kinematics::INTERPOLATION_SMOOTH));

    commands.push_back(new BodyJointCommand(time,gaitJoints,
											stiffness2,
                                            Kinematics::INTERPOLATION_SMOOTH));
    pthread_mutex_unlock(&walk_provider_mutex);
    return commands;
}

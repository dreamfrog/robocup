/*! @file HeadBehaviour.cpp
    @brief Head behaviour implementation for simpler behaviour control.

    @author Josiah Walker and Jake Fountain

 Copyright (c) 2012

 This file is free software: you can redistribute it and/or modify
 it under the terms of the GNU General Public License as published by
 the Free Software Foundation, either version 3 of the License, or
 (at your option) any later version.

 This file is distributed in the hope that it will be useful,
 but WITHOUT ANY WARRANTY; without even the implied warranty of
 MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 GNU General Public License for more details.

 You should have received a copy of the GNU General Public License
 along with NUbot.  If not, see <http://www.gnu.org/licenses/>.
 */
#include "HeadBehaviour.h"



#include <vector>
#include <cmath>


	    bool HeadBehaviour::ObjectNotSeen() {
			if (actionObjectID < 0) {
				return true;
			}
	        if (actionObjectID >= FieldObjects::NUM_MOBILE_FIELD_OBJECTS) {
                return ((actionStartTime > Blackboard->Objects->stationaryFieldObjects[actionObjectID - FieldObjects::NUM_MOBILE_FIELD_OBJECTS].TimeLastSeen())
                        and Blackboard->Objects->stationaryFieldObjects[actionObjectID - FieldObjects::NUM_MOBILE_FIELD_OBJECTS].TimeSeen()<CONFIRMATION_TIME);
	        } else if (actionObjectID >= 0) {
                return (actionStartTime > Blackboard->Objects->mobileFieldObjects[actionObjectID].TimeLastSeen() and
                        Blackboard->Objects->mobileFieldObjects[actionObjectID].TimeSeen() < CONFIRMATION_TIME);
            }
	        else return false;
	    }



        HeadBehaviour::HeadBehaviour():Mrlagent(){
            MAX_PERCEPT_RANGESIZE = 10;
            cout<<"head behaviour init started"<<endl;

	        NUCameraData cameraSpecs(string(CONFIG_DIR) + "CameraSpecs.cfg");
	        m_CAMERA_FOV_X = cameraSpecs.m_horizontalFov;
	        m_CAMERA_FOV_Y = cameraSpecs.m_verticalFov;
            head_logic = HeadLogic::getInstance();
            cout<<"head logic initialised"<<endl;
	        landmarkSeenFrequency = 1200;
	        ballSeenFrequency = 800;
	        lastVisionPolicy = -1;
	        maximumSearchTime = 2500.;
            CONFIRMATION_TIME = 250.;
            buttonPressTime = 0.;
	        actionObjectID = -1;
            time_since_last_localisation = 0;
            srand(Blackboard->Sensors->GetTimestamp());
            //MRL Agent
            vector<float> inputs = getPercept();
            cout<<"Initialising MRLAgent..."<<endl;

            //Mrlagent();

            try{
                cout<<"loading Headbehaviour agent"<<endl;
                Mrlagent.loadMRLAgent("HeadBehaviourRL");
                cout<<"HeadBehaviour::HeadBehaviour():Mrlagent() - agent loaded successfully."<<endl;
            } catch (string s){
                Mrlagent.log("Agent did not load properly."+s);
                cout<<s<<endl;

                Mrlagent.initialiseAgent(inputs.size()
                                         //Calculate output size = sum of the number of stationary, mobile and ambiguous objects.
                                         ,head_logic->relevantObjects[0].size()+head_logic->relevantObjects[1].size()+head_logic->relevantObjects[2].size()
                                         /*Order of fourier approx.*/
                                         ,1
                                         //max_percept_values
                                         ,MAX_PERCEPT_RANGESIZE);
                cout<<"setting Mrlagent params"<< endl;

                Mrlagent.setParameters(0.5,0.3,0.1,0.5,5,15, true);

            }

           //cout<<"Mrlagent initialised in head_behaviour: head behaviour init finished"<<endl;
        }

        HeadBehaviour::~HeadBehaviour(){
            Mrlagent.saveMRLAgent("HeadBehaviourMRL");
            cout<<"HeadBehaviour::~HeadBehaviour() - MRLAgent saved successfully"<<endl;
            delete head_logic;

        }

	    void HeadBehaviour::doPriorityListPolicy() {
        /*
            //Josiah's simple priority list head behaviour.
	    	//prepare vision priority times
	        //cout << "doing priority list policy" << flush;
	        float timeSinceBallSeen = Blackboard->Objects->mobileFieldObjects[FieldObjects::FO_BALL].TimeSinceLastSeen()+ballFocusBias;
	        float timeSinceLandmarkSeen = min(10000000.f,Blackboard->Objects->stationaryFieldObjects[FieldObjects::FO_BLUE_LEFT_GOALPOST ].TimeSinceLastSeen());
	        timeSinceLandmarkSeen = min(timeSinceLandmarkSeen,Blackboard->Objects->stationaryFieldObjects[FieldObjects::FO_BLUE_RIGHT_GOALPOST ].TimeSinceLastSeen());
	        timeSinceLandmarkSeen = min(timeSinceLandmarkSeen,Blackboard->Objects->stationaryFieldObjects[FieldObjects::FO_YELLOW_RIGHT_GOALPOST ].TimeSinceLastSeen());
	        timeSinceLandmarkSeen = min(timeSinceLandmarkSeen,Blackboard->Objects->stationaryFieldObjects[FieldObjects::FO_YELLOW_LEFT_GOALPOST ].TimeSinceLastSeen());
	        //cout << "Ball: " << timeSinceBallSeen << "; Landmark: " << timeSinceLandmarkSeen << endl;
	        //check what we need to prioritise looking at
	        if (timeSinceBallSeen/ballSeenFrequency > timeSinceLandmarkSeen/landmarkSeenFrequency) {
                dispatchHeadJob((Object)&Blackboard->Objects->mobileFieldObjects[FieldObjects::FO_BALL]);
                actionObjectID = Blackboard->Objects->mobileFieldObjects[FieldObjects::FO_BALL].getID() + FieldObjects::NUM_MOBILE_FIELD_OBJECTS;
	        } else {
               // dispatchHeadJob(&(Blackboard->Objects->stationaryFieldObjects[FieldObjects::]);
            }*/

	    }

        void HeadBehaviour::doTimeVSCostPriorityPolicy(){
            //Do highest priority of HeadLogic's interesting objects. Priority = (time since last seen)/cost

            vector<float> times = head_logic->getTimeSinceLastSeenSummary();
            vector<float> costs = head_logic->getCostList((float)1.0,(float)1.0);//Look at whole screen.


            //Debug:
            cout<<" HeadBehaviour::doPriorityListPolicy(): cost list = [";
            for(int i = 0; i<costs.size(); i++){
                cout<<" "<<costs[i]<<",";
            }
            cout<<"]"<<endl;
            cout<<" HeadBehaviour::doPriorityListPolicy(): time since last seen list = [";
            for(int i = 0; i<times.size(); i++){
                cout<<" "<<times[i]<<",";
            }
            cout<<"]"<<endl;

            vector<float> priorities(0);
            if(times.size()==costs.size()){
                for (int i = 0; i<times.size(); i++ ){
                    if(costs[i]==0.0){
                        priorities.push_back((float)0);//Don't look at something already in vision.
                    } else {
                        priorities.push_back((float)times[i]/costs[i]);
                    }
                }
            }else cout<<"HeadBehaviour::doPriorityListPolicy(): Times and costs different sizes()."<<endl;

            cout<<" HeadBehaviour::doPriorityListPolicy(): priorities = [";
            for(int i = 0; i<priorities.size(); i++){
                cout<<" "<<priorities[i]<<",";
            }
            cout<<"]"<<endl;


            float bestPriority = 0;
            int bestObject = 1;
            //Find index of best priority Object
            for (int i = 0; i< priorities.size();i++){
                if (bestPriority>priorities[i]){
                    bestPriority = priorities[i];
                    bestObject = i;
                }
            }

            int ob_type = head_logic->getObjectType(bestObject);

            if (ob_type == 1){
                dispatchHeadJob((MobileObject*)head_logic->getObject(bestObject));
                cout<<" HeadBehaviour::doPriorityListPolicy() Performing priority policy - Mobile Object: "<< bestObject <<endl;
            } else if (ob_type == 2){
                dispatchHeadJob((AmbiguousObject*)head_logic->getObject(bestObject));
                cout<<" HeadBehaviour::doPriorityListPolicy() Performing priority policy - Ambiguous Object: "<< bestObject <<endl;
            } else {
                dispatchHeadJob((StationaryObject*)head_logic->getObject(bestObject));
                cout<<" HeadBehaviour::doPriorityListPolicy() Performing priority policy - Stationary Object: "<< bestObject <<endl;
            }


            //Warning:temporary p
        }





        HeadBehaviour* HeadBehaviour::getInstance() {
	        //if (!Instance) {
	        static HeadBehaviour* Instance = new HeadBehaviour();
	        //}
	        return Instance;
	    }

	    //main function that drives choosing what to look at depending on the desired policy
        void HeadBehaviour::makeVisionChoice(VisionPolicyID fieldVisionPolicy) {

            float main_button_state;
            Blackboard->Sensors->getButton(NUSensorsData::LeftButton,main_button_state);
            float current_time = Blackboard->Sensors->GetTimestamp();
            /*Save button implementation:
            if((fieldVisionPolicy == RLAgentPolicy or fieldVisionPolicy == MRLAgentPolicy) and main_button_state >0 and buttonPressTime+2500.< current_time){
                Mrlagent.saveMRLAgent("HeadBehaviourRL");
                buttonPressTime = current_time;
            }*/

	        //If we are still moving to look at something else, don't make a new decision
            if (current_time < actionStartTime+maximumSearchTime &&
                fieldVisionPolicy == lastVisionPolicy and
                ObjectNotSeen()) {
	            //XXX: add a new head dispatch when things are standardised
                cout << "HeadBehaviour::makeVisionChoice  - Blocking on vision" << endl;
	            return;
	        }

/*

            //if we didn't see the object, we may need to relocalise
            if (fieldVisionPolicy!=TimeVSCostPriority and actionObjectID >= 0 and ObjectNotSeen() and fieldVisionPolicy == lastVisionPolicy and calculateReward()<-1000){
				Blackboard->Jobs->addMotionJob(new HeadPanJob(HeadPanJob::BallAndLocalisation));
				actionStartTime = Blackboard->Sensors->GetTimestamp()+4000;
				actionObjectID = -1;
                time_since_last_localisation = current_time;

                cout << "objects lost, searching" << endl;
                return;
            }*/



			//cout << "looking for something now" << endl;
			lastVisionPolicy = fieldVisionPolicy;

	        switch (fieldVisionPolicy) {
	            case BallFarVisionPolicy:
	                landmarkSeenFrequency = 1200;
	                ballSeenFrequency = 800;
	                ballFocusBias = 200.;
	                doPriorityListPolicy();
	                break;
	            case BallNearVisionPolicy:
	                landmarkSeenFrequency = 1400;
	                ballSeenFrequency = 600;
	                ballFocusBias = 800.;
	                doPriorityListPolicy();
	                break;
	            case BallLostVisionPolicy:
	            case RobotLostVisionPolicy:
	            case BallOnlyVisionPolicy:
	                landmarkSeenFrequency = 1000000.;
	                ballSeenFrequency = 10;
	                ballFocusBias = 1000.;
	                doPriorityListPolicy();
	                break;
	            case LandmarkOnlyVisionPolicy:
	                landmarkSeenFrequency = 10.;
	                ballSeenFrequency = 1000000.;
	                ballFocusBias = -1000.;
	                doPriorityListPolicy();
	                break;
                case TimeVSCostPriority:
                    doTimeVSCostPriorityPolicy();
                    break;
                case RLAgentPolicy:
                    doRLAgentPolicy();
                    break;
                case MRLAgentPolicy:
                    doMRLAgentPolicy();
                    break;

	        }
	    }


        void HeadBehaviour::doMRLAgentPolicy(){

            vector<float> inputs = getPercept();


            int action = Mrlagent.getActionAndLearn(inputs);//Should output integer from 0 to outputs-1


            if (action < head_logic->relevantObjects[0].size()){    //i.e. Stationary Object
                dispatchHeadJob((StationaryObject*)(head_logic->getObject(action)));
                cout<<" HeadBehaviour::doMRLAgentPolicy() Performing priority policy - Stationary Object: "<< action << "- Percept = [";
                /*for(int i = 0; i<inputs.size(); i++){
                    cout<<" "<<inputs[i]<<",";
                }*/
                cout<<"]"<<endl;
            } else if (action < head_logic->relevantObjects[1].size()+head_logic->relevantObjects[0].size()){ //i.e. Mobile Object
                dispatchHeadJob((MobileObject*)(head_logic->getObject(action)));
                cout<<" HeadBehaviour::doMRLAgentPolicy() Performing priority policy - Mobile Object: "<< action << "- Percept = [";
                /*for(int i = 0; i<inputs.size(); i++){
                    cout<<" "<<inputs[i]<<",";
                }*/
                cout<<"]"<<endl;
            } else {
                dispatchHeadJob((AmbiguousObject*)(head_logic->getObject(action)));
                cout<<" HeadBehaviour::doMRLAgentPolicy() Performing priority policy - Ambiguous Object: "<< action << "- Percept = [";
                /*for(int i = 0; i<inputs.size(); i++){
                    cout<<" "<<inputs[i]<<",";
                }*/
                cout<<"]"<<endl;
            }

        }

        void HeadBehaviour::doRLAgentPolicy(){
            vector<float> inputs = getPercept();
            int action = Mrlagent.getAction(inputs);
            float rew = calculateReward();
            cout<< "Reward = "<<rew<<endl;
            Mrlagent.giveReward(rew);
            Mrlagent.doLearning();
            Mrlagent.saveMRLAgent("HeadBehaviourRL");
            if (action < head_logic->relevantObjects[0].size()){    //i.e. Stationary Object
                dispatchHeadJob((StationaryObject*)(head_logic->getObject(action)));
                cout<<" HeadBehaviour::doRLAgentPolicy() Performing priority policy - Stationary Object: "<< action <<endl;
                cout<< "- Percept = [";
                for(int i = 0; i<inputs.size(); i++){
                    cout<<" "<<inputs[i]<<",";
                }
                cout<<"]"<<endl;
            } else if (action < head_logic->relevantObjects[1].size()+head_logic->relevantObjects[0].size()){ //i.e. Mobile Object
                dispatchHeadJob((MobileObject*)(head_logic->getObject(action)));
                cout<<" HeadBehaviour::doRLAgentPolicy() Performing priority policy - Mobile Object: "<< action << endl;
                cout<< "Percept = [";
                for(int i = 0; i<inputs.size(); i++){
                    cout<<" "<<inputs[i]<<",";
                }
                cout<<"]"<<endl;
            } else {
                dispatchHeadJob((AmbiguousObject*)(head_logic->getObject(action)));
                cout<<" HeadBehaviour::doRLAgentPolicy() Performing priority policy - Ambiguous Object: "<< action <<endl;
                cout<<  "- Percept = [";
                for(int i = 0; i<inputs.size(); i++){
                    cout<<" "<<inputs[i]<<",";
                }
                cout<<"]"<<endl;
            }
        }

        float HeadBehaviour::calculateReward(){
            //Sum square of the errors in self x, y and heading and errors in ball location.
            float errx = Blackboard->Objects->self.sdX();
            float erry = Blackboard->Objects->self.sdY();
            float errh = Blackboard->Objects->self.sdHeading();

            Vector2<float> berr = Blackboard->Objects->mobileFieldObjects[FieldObjects::FO_BALL].getEstimatedFieldLocationError();
            float ball_reward = -(1/(1+exp(-(berr.x*berr.x+berr.y*berr.y)/500)));
            float self_reward = -(1/(1+exp(-(errx*errx+erry*erry+errh*errh)/500)));
            if(ball_reward < -0.99) return self_reward;//Ball is lost
            return (ball_reward+self_reward)/2;
        }

        vector<float> HeadBehaviour::getPercept(){
           //OLD INPUTS START:
/*
            //Note for future use: If the percept vectors are changed, the RLagent input size must be changed.
            vector<float> inputs= head_logic->getTimeSinceLastSeenSummary();
            //Scaling inputs:
            for (int i = 0; i<inputs.size(); i++){
                inputs[i] = MAX_PERCEPT_RANGESIZE*(1-exp(-inputs[i]/100));
            }
            vector<float> other_inputs = head_logic->getCostList(0,0);//Costs to look at objects
            for (int i = 0; i<other_inputs.size(); i++){
                other_inputs[i] = MAX_PERCEPT_RANGESIZE*(1-exp(-other_inputs[i]/4));
            }
            vector<float> locations = head_logic->getSimplePolarObLocSummary();//absolute self location and relative locations of objects.
            //Scaling inputs:
            for (int i = 0; i<locations.size(); i++){
                if(i%2 == 0 and i!=2){
                    locations[i] = MAX_PERCEPT_RANGESIZE*(1-exp(-locations[i]*locations[i]/5));
                }else{
                    locations[i] =  (MAX_PERCEPT_RANGESIZE/(1+exp(-locations[i]/100))-MAX_PERCEPT_RANGESIZE/2.);
                }
            }
            //Combines inputs to one vector to feed to RLagent
            inputs.insert(inputs.end(), other_inputs.begin(), other_inputs.end());
            inputs.insert(inputs.end(), locations.begin(), locations.end());
            //OLD INPUTS END
*/

            vector<float> inputs= head_logic->getTimeSinceLastSeenSummary();
            vector<float> costs = head_logic->getCostList(0,0);
            for(int i = 0;i<inputs.size();i++){
                inputs[i]=inputs[i]*(1+exp(-(costs[i]-8)/4));//Divide times by sigmoided costs to get priorities.
            }

            for (int i = 0; i<inputs.size(); i++){
                inputs[i] = MAX_PERCEPT_RANGESIZE*(1-exp(-inputs[i]/200.));
            }
            vector<float> self_location = head_logic->getSelfLocation();
            vector<float> ball_location(2,0);

            ball_location[0] = Blackboard->Objects->mobileFieldObjects[0].X();
            ball_location[1] = Blackboard->Objects->mobileFieldObjects[0].Y();

            for (int i = 0; i<self_location.size(); i++){
                self_location[i] =  (MAX_PERCEPT_RANGESIZE/(1+exp(-self_location[i]/100.))-MAX_PERCEPT_RANGESIZE/2.);
                ball_location[i] =(MAX_PERCEPT_RANGESIZE/(1+exp(-ball_location[i]/100.))-MAX_PERCEPT_RANGESIZE/2.);
            }

            float h = Blackboard->Objects->self.Heading();
            inputs.push_back(MAX_PERCEPT_RANGESIZE*(1-exp(-h*h/6.)));
            inputs.insert(inputs.end(), self_location.begin(), self_location.end());
            inputs.insert(inputs.end(), ball_location.begin(), ball_location.end());

            float errx = Blackboard->Objects->self.sdX();
            float erry = Blackboard->Objects->self.sdY();
            float errh = Blackboard->Objects->self.sdHeading();

            Vector2<float> berr = Blackboard->Objects->mobileFieldObjects[FieldObjects::FO_BALL].getEstimatedFieldLocationError();
            float ball_err = (MAX_PERCEPT_RANGESIZE*(1-exp(-(berr.x*berr.x+berr.y*berr.y)/500.)));
            float self_err= (MAX_PERCEPT_RANGESIZE*(1-exp(-(errx*errx+erry*erry+errh*errh)/500.)));
            inputs.push_back(self_err);
            inputs.push_back(ball_err);
            return inputs;

             //0.327002, 151.674, -2.84274, 468.284, 0.00445226, 522.973, 0.00880493, 302.476, 0.0176379, 193.082, 0.0115795, 260.522, 0.000271216
        }

        void HeadBehaviour::dispatchHeadJob(StationaryObject* ObjectToTrack) {
            //initiate a new pan job using the robots estimated standard deviation of heading as the pan width

            actionObjectID = ObjectToTrack->getID();
            actionStartTime = Blackboard->Sensors->GetTimestamp();
            if (ObjectToTrack->isObjectVisible()) {
                Blackboard->Jobs->addMotionJob(new HeadTrackJob(*ObjectToTrack, Blackboard->Objects->self.sdHeading()));
            } else {
                Blackboard->Jobs->addMotionJob(new HeadPanJob(*ObjectToTrack, Blackboard->Objects->self.sdHeading()));
            }
        }
        void HeadBehaviour::dispatchHeadJob(MobileObject* ObjectToTrack) {
	        //initiate a new pan job using the robots estimated standard deviation of heading as the pan width
	        actionObjectID = ObjectToTrack->getID() + FieldObjects::NUM_MOBILE_FIELD_OBJECTS;
	        actionStartTime = Blackboard->Sensors->GetTimestamp();
	        if (ObjectToTrack->isObjectVisible()) {
				Blackboard->Jobs->addMotionJob(new HeadTrackJob(*ObjectToTrack, Blackboard->Objects->self.sdHeading()));
			} else {
				Blackboard->Jobs->addMotionJob(new HeadPanJob(*ObjectToTrack, Blackboard->Objects->self.sdHeading()));
            }
	    }
        void HeadBehaviour::dispatchHeadJob(AmbiguousObject* ObjectToTrack) {

            //initiate a new pan job using the robots estimated standard deviation of heading as the pan width
            actionObjectID = ObjectToTrack->getID() + FieldObjects::NUM_AMBIGUOUS_FIELD_OBJECTS;
            actionStartTime = Blackboard->Sensors->GetTimestamp();
            /*if (ObjectToTrack->isObjectVisible()) {
                Blackboard->Jobs->addMotionJob(new HeadTrackJob(*ObjectToTrack, Blackboard->Objects->self.sdHeading()));
            } else {
                Blackboard->Jobs->addMotionJob(new HeadPanJob(*ObjectToTrack, Blackboard->Objects->self.sdHeading()));
            }*/

        }

	    //XXX: head jobs do not map from localisation space back to vision space properly, fix in here.
	    /*void fieldXYToHeadingElevation(Vector2<float>* fieldposition, Vector2<float>* cameraposition) {

	    }*/



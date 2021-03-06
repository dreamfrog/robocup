#ifndef SELF_LOCWM_H_DEFINED
#define SELF_LOCWM_H_DEFINED
#include "Models/SelfSRUKF.h"
#include "Models/SelfUKF.h"
#include "Tools/Math/Filters/MobileObjectUKF.h"

#include "Infrastructure/FieldObjects/FieldObjects.h"
#include "Infrastructure/GameInformation/GameInformation.h"
class NUSensorsData;
#include "Infrastructure/TeamInformation/TeamInformation.h"

#include "debug.h"
#include "debugverbositylocalisation.h"
#include "Tools/FileFormats/TimestampedData.h"
#include "Tools/Math/Vector2.h"
#include <fstream>
#include <sstream>
#include <list>

// Debug output level.
// Please follow this guide.
// 0 - No messages
// 1 - Error messages
// 2 - Update messages
// 3 - All messages
// #define  DEBUG_LOCALISATION_VERBOSITY 3

//#define LOC_SUMMARY 3

typedef SelfSRUKF Model;
//typedef SelfUKF Model;
typedef std::list<SelfModel*> ModelContainer;
typedef std::pair<unsigned int, float> ParentSum;
#include "LocalisationSettings.h"

class SelfLocalisation: public TimestampedData
{
    public:
        SelfLocalisation(int playerNumber = 0);
        SelfLocalisation(int playerNumber, const LocalisationSettings& settings);
        SelfLocalisation(const SelfLocalisation& source);
        ~SelfLocalisation();
    
        void process(NUSensorsData* data, FieldObjects* fobs, const GameInformation* gameInfo, const TeamInformation* teamInfo);
	
        void ProcessObjects(FieldObjects* fobs, float time_increment);
        void writeToLog();
        bool doTimeUpdate(float odomForward, float odomLeft, float odomTurn, double time_increment);
        void WriteModelToObjects(const SelfModel* model, FieldObjects* fobs);
        bool clipModelToField(SelfModel* theModel);
        bool clipActiveModelsToField();

        int multipleLandmarkUpdate(std::vector<StationaryObject*>& landmarks);
        int landmarkUpdate(StationaryObject &landmark);
        bool ballUpdate(const MobileObject& ball);
        bool sharedBallUpdate(const std::vector<TeamPacket::SharedBall>& sharedBalls);

        // Ambiguous object updates.
        // Main function.
        int ambiguousLandmarkUpdate(AmbiguousObject &ambigousObject, const vector<StationaryObject*>& possibleObjects);
        // Individual methods.
        int ambiguousLandmarkUpdateExhaustive(AmbiguousObject &ambigousObject, const vector<StationaryObject*>& possibleObjects);
        int ambiguousLandmarkUpdateSelective(AmbiguousObject &ambigousObject, const vector<StationaryObject*>& possibleObjects);
        int ambiguousLandmarkUpdateProbDataAssoc(AmbiguousObject &ambigousObject, const vector<StationaryObject*>& possibleObjects);
        int ambiguousLandmarkUpdateConstraint(AmbiguousObject &ambiguousObject);


        int doTwoObjectUpdate(StationaryObject &landmark1, StationaryObject &landmark2);
        unsigned int getNumActiveModels();
        unsigned int getNumFreeModels();
        const SelfModel* getBestModel() const;
        const MobileObjectUKF* getBallModel() const;
        void NormaliseAlphas();
        int FindNextFreeModel();

        // Pruning functions.
        // Main function.
        int PruneModels();
        // Individual methods.
        int PruneMaxLikelyhood();
        int PruneViterbi(unsigned int order);
        int PruneNScan(unsigned int N);
        void MergeModels(int maxAfterMerge);

        // Merging helper functions.
        bool MergeTwoModels(SelfModel* modelA, SelfModel* modelB);
        double MergeMetric(const SelfModel* modelA, const SelfModel* modelB) const;
        void MergeModelsBelowThreshold(double MergeMetricThreshold);
        void PrintModelStatus(const SelfModel* model);
        std::string ModelStatusSummary();

        void removeAmbiguousGoalPairs(std::vector<AmbiguousObject>& ambiguousobjects, bool yellow_seen, bool blue_seen);

        void resetPlayingStateModels();

        bool m_amILost;                       // true if we are 'lost' in this frame
        int m_lostCount;                      // the number of consecutive frames in which we are 'lost'
        float m_timeSinceFieldObjectSeen;     // the time since a useful field object has been seen

        unsigned int removeInactiveModels();
        static unsigned int removeInactiveModels(ModelContainer& container);
        const ModelContainer allModels() const
        {return m_models;}

        // Model Reset Functions
        void initSingleModel(float x, float y, float heading);
        void initBallModel(MobileObjectUKF* ball_model);
        bool CheckGameState(bool currently_incapacitated, const GameInformation *game_info);
        void doInitialReset(GameInformation::TeamColour team_colour);
        void doSingleInitialReset(GameInformation::TeamColour team_colour);
        void doSetReset(GameInformation::TeamColour team_colour, int player_number, bool have_kickoff);
        void doPenaltyReset();
        void doBallOutReset();
        void doFallenReset();
        void doReset();
        void resetSdMatrix(int modelNumber);
        void swapFieldStateTeam(float& x, float& y, float& heading);

        static Matrix mean_matrix(float x, float y, float heading);
        static Matrix covariance_matrix(float x_var, float y_var, float heading_var);
        void InitialiseModels(const std::vector<Moment>& positions);
        void setModels(ModelContainer& newModels);

        void addToBallVariance(float x_pos_var, float y_pos_var, float x_vel_var, float y_vel_var);
        void setBallVariance(float x_pos_var, float y_pos_var, float x_vel_var, float y_vel_var);

        void clearModels();

        std::vector<TeamPacket::SharedBall> FindNewSharedBalls(const std::vector<TeamPacket::SharedBall>& allSharedBalls);

        std::string frameLog() const
        {
            return m_frame_log.str();
        }

        // Implementation of virtual function from TimestampedData class.
        double GetTimestamp() const {return m_timestamp;}

        static char header(){return 's';}

        bool operator ==(const SelfLocalisation& b) const;
        bool operator !=(const SelfLocalisation& b) const
        {return (!((*this) == b));}

        /*!
        @brief Outputs a binary representation of the Self Localisation system.
        @param output The output stream.
        @return The output stream.
        */
        std::ostream& writeStreamBinary (std::ostream& output) const;

        /*!
        @brief Reads in a Self localisation system from the input stream.
        @param input The input stream.
        @return The input stream.
        */
        std::istream& readStreamBinary (std::istream& input);

        /*!
        @brief Output streaming operation.
        @param output The output stream.
        @param p_loc The source localisation data to be streamed.
        */
        friend std::ostream& operator<< (std::ostream& output, const SelfLocalisation& p_loc);

        /*!
        @brief Input streaming operation.
        @param input The input stream.
        @param p_kf The destination localisation data to be streamed to.
        */
        friend std::istream& operator>> (std::istream& input, SelfLocalisation& p_loc);

        SelfLocalisation& operator= (const SelfLocalisation & source);

    protected:
        MeasurementError calculateError(const Object& theObject);
        Vector2<float> TriangulateTwoObject(const StationaryObject& object1, const StationaryObject& object2);

        // Multiple Models Stuff
        static const int c_MAX_MODELS_AFTER_MERGE = 6; // Max models at the end of the frame
        static const int c_MAX_MODELS = (c_MAX_MODELS_AFTER_MERGE*8+2); // Total models
        ModelContainer m_models;
        MobileObjectUKF* m_ball_model;

	#if DEBUG_LOCALISATION_VERBOSITY > 0
        ofstream debug_file; // Logging file
        #endif // LOCWM_VERBOSITY > 0

        double m_timestamp;
        int m_currentFrameNumber;
        double m_prev_ball_update_time;

        // Game state memory
        bool m_previously_incapacitated;
        GameInformation::RobotState m_previous_game_state;
        std::stringstream m_frame_log;

        std::vector<float> m_gps;
        float m_compass;
        bool m_hasGps;
        
        // Settings
        LocalisationSettings m_settings;

        std::vector<AmbiguousObject> m_pastAmbiguous;
        std::vector<TeamPacket::SharedBall> m_prevSharedBalls;

        // Outlier tuning Constants -- Values assigned in SelfLocalisation.cpp
        static const float c_LargeAngleSD;

        // Object distance measurement error weightings (Constant) -- Values assigned in SelfLocalisation.cpp
        static const float c_obj_theta_variance;
        static const float c_obj_range_offset_variance;
        static const float c_obj_range_relative_variance;
        static const float c_centre_circle_heading_variance;
        static const float c_twoObjectAngleVariance;
};

#endif

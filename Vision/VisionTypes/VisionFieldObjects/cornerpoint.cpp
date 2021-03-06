#include "cornerpoint.h"
#include "Vision/visionblackboard.h"
#include "debug.h"
#include "debugverbosityvision.h"

CornerPoint::CornerPoint(TYPE type, GroundPoint location)
{
    m_size_on_screen = Vector2<double>(3,3);
    m_type = type;
    m_location = location;
    const Transformer& t = VisionBlackboard::getInstance()->getTransformer();

    // calculate bearing and elevation
    t.screenToRadial2D(m_location);
    valid = t.isDistanceToPointValid();
    if(valid) {
        // find distance
        double distance = t.distanceToPoint(m_location.angular.x, m_location.angular.y);
        // calculate foot relative 3D location
        t.radial2DToRadial3D(m_location, distance);
    }
}

bool CornerPoint::addToExternalFieldObjects(FieldObjects* fieldobjects, float timestamp) const
{
#if VISION_FIELDPOINT_VERBOSITY > 1
    debug << "CornerPoint::addToExternalFieldObjects - m_id: " << VFOName(m_id) << endl;
    debug << "    " << *this << endl;
#endif
    if(valid) {
        #if VISION_FIELDPOINT_VERBOSITY > 1
            debug << "CornerPoint::addToExternalFieldObjects - valid" << endl;
        #endif
        AmbiguousObject newAmbObj;

        switch(m_type) {
        case L:
            // labelling inside L since I have no idea which is which - going to match it with other possibilities
            newAmbObj = AmbiguousObject(FieldObjects::FO_CORNER_UNKNOWN_INSIDE_L, "L Corner");
            newAmbObj.addPossibleObjectID(FieldObjects::FO_CORNER_YELLOW_FIELD_LEFT);
            newAmbObj.addPossibleObjectID(FieldObjects::FO_CORNER_YELLOW_FIELD_RIGHT);
            newAmbObj.addPossibleObjectID(FieldObjects::FO_CORNER_YELLOW_PEN_LEFT);
            newAmbObj.addPossibleObjectID(FieldObjects::FO_CORNER_YELLOW_PEN_RIGHT);
            newAmbObj.addPossibleObjectID(FieldObjects::FO_CORNER_CENTRE_CIRCLE);
            newAmbObj.addPossibleObjectID(FieldObjects::FO_CORNER_BLUE_PEN_LEFT);
            newAmbObj.addPossibleObjectID(FieldObjects::FO_CORNER_BLUE_PEN_RIGHT);
            newAmbObj.addPossibleObjectID(FieldObjects::FO_CORNER_BLUE_FIELD_LEFT);
            newAmbObj.addPossibleObjectID(FieldObjects::FO_CORNER_BLUE_FIELD_RIGHT);

            break;
        case T:
            newAmbObj = AmbiguousObject(FieldObjects::FO_CORNER_UNKNOWN_T, "T Corner");
            newAmbObj.addPossibleObjectID(FieldObjects::FO_CORNER_YELLOW_T_LEFT);
            newAmbObj.addPossibleObjectID(FieldObjects::FO_CORNER_YELLOW_T_RIGHT);
            newAmbObj.addPossibleObjectID(FieldObjects::FO_CORNER_CENTRE_T_LEFT);
            newAmbObj.addPossibleObjectID(FieldObjects::FO_CORNER_CENTRE_T_RIGHT);
            newAmbObj.addPossibleObjectID(FieldObjects::FO_CORNER_BLUE_T_LEFT);
            newAmbObj.addPossibleObjectID(FieldObjects::FO_CORNER_BLUE_T_RIGHT);
            newAmbObj.addPossibleObjectID(FieldObjects::FO_CORNER_PROJECTED_T_YELLOW_LEFT);
            newAmbObj.addPossibleObjectID(FieldObjects::FO_CORNER_PROJECTED_T_YELLOW_RIGHT);
            newAmbObj.addPossibleObjectID(FieldObjects::FO_CORNER_PROJECTED_T_BLUE_LEFT);
            newAmbObj.addPossibleObjectID(FieldObjects::FO_CORNER_PROJECTED_T_BLUE_RIGHT);
            break;
        case X:
            // At present a cross is ignored as none exist on the field and this indicates poor detection
            return false;
        default:
            //invalid object - do not push to fieldobjects
            errorlog << "CornerPoint::addToExternalFieldObjects - attempt to add invalid CornerPoint object id: " << VFOName(m_id) << endl;
            #if VISION_FIELDPOINT_VERBOSITY > 1
                debug << "CornerPoint::addToExternalFieldObjects - attempt to add invalid CornerPoint object id: " << VFOName(m_id) << endl;
            #endif
            return false;
        }

        //update ambiguous corner and add it to ambiguousFieldObjects
        newAmbObj.UpdateVisualObject(Vector3<float>(m_location.relativeRadial.x, m_location.relativeRadial.y, m_location.relativeRadial.z),
                                     Vector3<float>(m_spherical_error.x, m_spherical_error.y, m_spherical_error.z),
                                     Vector2<float>(m_location.angular.x, m_location.angular.y),
                                     Vector2<int>(m_location.screen.x,m_location.screen.y),
                                     Vector2<int>(m_size_on_screen.x,m_size_on_screen.y),
                                     timestamp);
        fieldobjects->ambiguousFieldObjects.push_back(newAmbObj);

        return true;
    }
    else {
        #if VISION_FIELDPOINT_VERBOSITY > 1
            debug << "CornerPoint::addToExternalFieldObjects - invalid" << endl;
        #endif
        return false;
    }
}

//! @brief Stream output for labelling purposes
void CornerPoint::printLabel(ostream& out) const
{
    out << m_location;
}

//! @brief Calculation of error for optimisation
double CornerPoint::findScreenError(VisionFieldObject* other) const
{
    CornerPoint* c = dynamic_cast<CornerPoint*>(other);
    return ( m_location.screen - c->m_location.screen ).abs();
}

double CornerPoint::findGroundError(VisionFieldObject *other) const
{
    CornerPoint* c = dynamic_cast<CornerPoint*>(other);
    return ( m_location.ground - c->m_location.ground ).abs();
}

ostream& operator<< (ostream& output, const CornerPoint& c)
{
    string nm;
    switch(c.m_type) {
    case CornerPoint::L:
        nm = "L";
        break;
    case CornerPoint::T:
        nm = "T";
        break;
    case CornerPoint::X:
        nm = "X";
        break;
    default:
        nm = "INVALID";
        break;
    }

    output << "CornerPoint - " << nm << endl;
    output << "\tpixelloc: " << c.m_location.screen << endl;
    output << "\tangularloc: " << c.m_location.angular << endl;
    output << "\trelative field coords: " << c.m_location.relativeRadial << endl;
    output << "\tspherical error: [" << c.m_spherical_error << "]" << endl;
    output << "\tsize on screen: [" << c.m_size_on_screen << "]";
    return output;
}

ostream& operator<< (ostream& output, const vector<CornerPoint>& c)
{
    for (size_t i=0; i<c.size(); i++)
        output << c[i] << endl;
    return output;
}

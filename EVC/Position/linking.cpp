#include "linking.h"
#include "distance.h"
#include "../Supervision/national_values.h"
#include "../Packets/messages.h"
std::list<link_data> linking;
int NID_LRBG;
double Q_LOCACC_LRBG;
double update_location_reference(int nid_bg, distance group_pos, bool linked)
{
    if (!linked) {
        double offset = group_pos.get();
        distance::update_unlinked_reference(offset);
        return offset;
    } else if (linking.empty()) {
        double offset = group_pos.get();
        reset_odometer(offset);
        distance::update_distances(offset, offset);
        NID_LRBG = nid_bg;
        Q_LOCACC_LRBG = Q_NVLOCACC;
        return 0;
    } else if (linking.begin()->nid_bg==nid_bg) {
        double offset = linking.begin()->dist.get();
        reset_odometer(offset);
        distance::update_distances(offset, group_pos.get());
        NID_LRBG = nid_bg;
        Q_LOCACC_LRBG = linking.front().locacc;
        group_pos = distance(0);
        return 0;
    }
    //Never reach this point
    return group_pos.get();
}
void update_linking(distance start, Linking link, bool infill)
{
    std::vector<LinkingElement> elements;
    elements.push_back(link.element);
    elements.insert(elements.end(), link.elements.begin(), link.elements.end());
    std::list<link_data> links;
    distance cumdist=start;
    for (LinkingElement l : elements)
    {
        link_data d;
        d.dist = cumdist+l.D_LINK.get_value(link.Q_SCALE);
        d.locacc = l.Q_LOCACC;
        d.nid_bg = l.NID_BG;
        d.reaction = l.Q_LINKREACTION;
        d.reverse_dir = l.Q_LINKORIENTATION == Q_LINKORIENTATION_t::Reverse;
        links.push_back(d);
        cumdist = d.dist;
    }
    if (infill) {
        linking.erase(linking.begin(), link_expected);
        for (auto it = linking.begin(); it!=linking.end(); ++it) {
            if (it->dist > start) {
                linking.erase(it, linking.end());
                break;
            }
        }
        linking.insert(linking.end(), links.begin(), links.end());
    } else {
        linking = links;
    }
    link_expected = linking.begin();
}
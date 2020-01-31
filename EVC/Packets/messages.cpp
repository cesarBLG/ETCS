#include "messages.h"
#include "12.h"
#include "21.h"
#include "27.h"
#include "136.h"
static int reading_nid_bg=-1;
static std::vector<eurobalise_telegram> telegrams;
static distance last_passed_distance;
static bool reffound=false;
static bool refmissed=false;
static bool dupfound=false;
static bool refpassed=false;
static bool linked = false;
static int prevpig=-1;
static int totalbg=8;
static bool reading = false;
static int dir = -1;
static distance bg_reference1;
static distance bg_reference1max;
static distance bg_reference1min;
static distance bg_reference;
static distance bg_referencemax;
static distance bg_referencemin;
static bool stop_checking_linking=false;
std::list<link_data>::iterator link_expected=linking.end();
std::deque<eurobalise_telegram> pending_telegrams;
void trigger_reaction(int reaction);
void handle_packet(ETCS_packet *p, distance dist, bool infill, int dir);
void handle_telegrams(std::vector<eurobalise_telegram> message, distance dist, int dir);
void check_valid_data(std::vector<eurobalise_telegram> telegrams, distance bg_reference, bool linked);
void check_eurobalise_passed();
void balise_group_passed();
void check_linking();
void expect_next_linking();
void trigger_reaction(int reaction)
{
    switch (reaction) {
        case 1:
            std::cout<<"TODO: Service brake due to balise read error"<<std::endl;
            break;
        case 2:
            //No reaction
            break;
        default:
            std::cout<<"TODO: Train Trip - Balise read error"<<std::endl;
            break;
    }
}
void reset_eurobalise_data()
{
    telegrams.clear();
    reading_nid_bg = -1;
    prevpig = -1;
    totalbg = 8;
    reading = false;
    dir = -1;
    refmissed = false;
    reffound = false;
    dupfound = false;
    refpassed = true;
    linked = false;
    stop_checking_linking = false;
}
void expect_next_linking()
{
    if (link_expected != linking.end())
        ++link_expected;
}
void check_linking()
{
    if (link_expected!=linking.end() && !stop_checking_linking) {
        auto link_bg=linking.end();
        for (auto it = link_expected; it!=linking.end(); ++it) {
            if (it->nid_bg == reading_nid_bg) {
                link_bg = it;
                break;
            }
        }
        bool isexpected = linked && refpassed && link_expected==link_bg;
        bool c1 = isexpected && link_expected->min() > bg_referencemax;
        bool c2 = link_expected->max() < d_minsafefront(0);
        bool c3 = linked && link_bg!=linking.end() && link_bg != link_expected;
        if (c1 || c2 || c3) {
            trigger_reaction(link_expected->reaction);
            expect_next_linking();
            if (c3)
                check_linking();
        }
        if (isexpected) {
            stop_checking_linking = true;
            if (bg_referencemax>=link_expected->min() && bg_referencemin<=link_expected->max())
                expect_next_linking();
        }
    }
}
void balise_group_passed()
{
    if (!reffound) {
        refmissed = true;
        if (dupfound) {
            bg_reference = bg_reference1;
            bg_referencemax = bg_reference1max;
            bg_referencemin = bg_reference1min;
            refpassed = true;
            check_linking();
        }
    }
    bool linking_rejected=false;
    if (linked && reffound && !linking.empty()) {
        linking_rejected = true;
        for (link_data l : linking) {
            if (l.nid_bg == reading_nid_bg) {
                if (dir != -1 && dir != l.reverse_dir)
                    trigger_reaction(0);
                if (l.max() >= bg_referencemin && l.min() <= bg_referencemax)
                    linking_rejected = false;
                break;
            }
        }
    }
    if (!linking_rejected) check_valid_data(telegrams, bg_reference, linked);
    linking.erase(linking.begin(), link_expected);
    reset_eurobalise_data();
}
void check_eurobalise_passed()
{
    check_linking();
    if (reading) {
        double elapsed = d_estfront-last_passed_distance;
        if (prevpig==-1) {
            if (elapsed > 12*8)
                balise_group_passed();
        } else {
            int v1 = (totalbg-prevpig-1);
            int v2 = prevpig;
            if (dir == 0 && elapsed > v1*12)
                balise_group_passed();
            else if (dir == 1 && elapsed > v2*12)
                balise_group_passed();
            else if (dir == -1 && elapsed > std::max(v1, v2)*12)
                balise_group_passed();
        }
    }
    if (pending_telegrams.empty()) return;
    eurobalise_telegram t = pending_telegrams.front();
    pending_telegrams.pop_front();
    last_passed_distance = d_estfront;
    reading = true;
    if (!t.readerror) {
        linked = t.Q_LINK == Q_LINK_t::Linked;
        if (reading_nid_bg != -1 && reading_nid_bg != t.NID_BG)
            balise_group_passed();
        reading_nid_bg = t.NID_BG;
        totalbg = t.N_TOTAL+1;
        if (prevpig != -1) {
            if (dir == -1)
                dir = (prevpig>t.N_PIG) ? 1 : 0;
            if (t.N_PIG>prevpig && !reffound)
                refmissed = true;
        }
        prevpig = t.N_PIG;
        if (t.N_PIG == 1 && t.M_DUP == M_DUP_t::DuplicateOfPrev) {
            dupfound = true;
            bg_reference1 = d_estfront;
            bg_reference1max = d_maxsafefront(0);
            bg_reference1min = d_minsafefront(0);
        }
        if (t.N_PIG == 0) {
            reffound = true;
            bg_reference = d_estfront;
            bg_referencemax = d_maxsafefront(0);
            bg_referencemin = d_minsafefront(0);
            refpassed = true;
        }
        if ((dir==0 && t.N_PIG == t.N_TOTAL) || (dir == 1 && t.N_PIG == 0)) {
            telegrams.push_back(t);
            balise_group_passed();
            return;
        }
    }
    telegrams.push_back(t);
    if (refmissed && dupfound) {
        bg_reference = bg_reference1;
        bg_referencemax = bg_reference1max;
        bg_referencemin = bg_reference1min;
        refpassed = true;
    }
}
void check_valid_data(std::vector<eurobalise_telegram> telegrams, distance bg_reference, bool linked)
{
    int nid_bg=-1;
    int dir=-1;
    int prevno=-1;
    std::vector<eurobalise_telegram> read_telegrams;
    for (int i=0; i<telegrams.size(); i++) {
        eurobalise_telegram t = telegrams[i];
        if (!t.readerror) {
            nid_bg = t.NID_BG;
            if (prevno == -1)
                prevno = t.N_PIG;
            else
                dir = t.N_PIG<prevno; 
            read_telegrams.push_back(t);
        }
    }
    if (dir == 1)
        std::reverse(read_telegrams.begin(), read_telegrams.end());

    bool accepted1 = true;
    if (read_telegrams.size() == 0)
        accepted1 = false;

    link_data balise_link;
    bool containedinlinking=false;
    if (linked && !linking.empty()) {
        for (link_data l : linking) {
            if (l.nid_bg == nid_bg) {
                containedinlinking = true;
                balise_link = l;
            }
        }
        if (!containedinlinking)
            return;
    }

    std::set<int> missed;
    for (int i=0; i<read_telegrams.size(); i++) {
        eurobalise_telegram t = read_telegrams[i];
        if (i==0 && t.N_PIG != 0) {
            for (int j=0; j<t.N_PIG; j++)
                missed.insert(j);
        }
        if ((i>1 && telegrams[i-1].N_PIG+1<t.N_PIG)) {
            for (int j=telegrams[i-1].N_PIG; j<t.N_PIG; j++)
                missed.insert(j);
        }
        if (i+1==read_telegrams.size() && t.N_PIG!=t.N_TOTAL) {
            for (int j=t.N_PIG; j<=t.N_TOTAL; j++)
                missed.insert(j);
        }
    }
    for (int pig : missed) {
        bool reject=true;
        for (int i=0; i<read_telegrams.size(); i++) {
            eurobalise_telegram t = read_telegrams[i];
            if ((t.M_DUP == M_DUP_t::DuplicateOfNext && t.N_PIG+1==pig) || (t.M_DUP == M_DUP_t::DuplicateOfPrev && t.N_PIG==pig+1)) {
                if (containedinlinking) {
                    reject = false;
                } else {
                    if (dir!=-1) {
                        reject = false;
                    } else {
                        bool directional = false;
                        for (int j=0; j<t.packets.size(); j++) {
                            ETCS_packet *p = t.packets[i].get();
                            if (p->directional && ((ETCS_directional_packet*)p)->Q_DIR != Q_DIR_t::Both)
                                directional = true;
                        }
                        if (!directional)
                            reject = false;
                    }
                }
                break;
            }
        }
        if (reject)
            accepted1 = false;
    }
    std::vector<eurobalise_telegram> message;
    for (int i=0; i<read_telegrams.size(); i++) {
        eurobalise_telegram t = read_telegrams[i];
        bool c1 = t.M_DUP == M_DUP_t::NoDuplicates;
        bool c2 = t.M_DUP == M_DUP_t::DuplicateOfNext && i+1<read_telegrams.size() && t.N_PIG+1==read_telegrams[i+1].N_PIG;
        bool c3 = t.M_DUP == M_DUP_t::DuplicateOfPrev && i>1 && t.N_PIG==read_telegrams[i-1].N_PIG+1;
        if (c1 || !(c2||c3))
            message.push_back(t);
        if ((c2 && dir==0) || (c3 && dir==1)) {
            eurobalise_telegram first = t;
            eurobalise_telegram second = c2 ? read_telegrams[i+1] : read_telegrams[i-1];
            bool seconddefault = false;
            for (int j=0; j<second.packets.size(); j++) {
                if (second.packets[j]->NID_PACKET == 254) {
                    seconddefault = true;
                    break;
                }
            }
            message.push_back(seconddefault ? first : second);
        }
    }
    bool accepted2=true;
    int mcount=-1;
    for (int i=0; i<message.size(); i++) {
        if (!message[i].valid)
            accepted2 = false;
        if (message[i].M_MCOUNT==M_MCOUNT_t::NeverFitsTelegrams) {
            accepted2 = false;
        } else if(message[i].M_MCOUNT!=M_MCOUNT_t::FitsAllTelegrams) {
            if (mcount==-1)
                mcount = message[i].M_MCOUNT;
            else if (mcount != message[i].M_MCOUNT)
                accepted2 = false;
        }
    }
    bool accepted = accepted1 && accepted2;
    if (!accepted) {
        if (containedinlinking) {
            trigger_reaction(balise_link.reaction);
        } else {
            if (accepted2) {
                for (int i=0; i<message.size(); i++) {
                    eurobalise_telegram t = message[i];
                    for (int j=0; j<t.packets.size()-1; j++) {
                        auto p = t.packets[j];
                        if (p->NID_PACKET == 145) {
                            return;
                        }
                    }
                }
            }
            trigger_reaction(1);
        }
        return;
    }
    
    bg_reference = distance(0,update_location_reference(nid_bg, bg_reference, linked));
    if (dir == -1 && containedinlinking)
        dir = balise_link.reverse_dir;
    handle_telegrams(message, bg_reference, dir);
}
void handle_telegrams(std::vector<eurobalise_telegram> message, distance dist, int dir)
{
    for (int i=0; i<message.size(); i++) {
        eurobalise_telegram t = message[i];
        distance ref = dist;
        bool infill=false;
        for (int j=0; j<t.packets.size(); j++) {
            ETCS_packet *p = t.packets[j].get();
            if (p->NID_PACKET == 136) {
                InfillLocationReference ilr = *((InfillLocationReference*)p);
                bool found = false;
                for (link_data l : linking) {
                    if (l.nid_bg == ilr.NID_BG) {
                        found = true;
                        infill = true;
                        ref = l.dist;
                        break;
                    }
                }
                if (!found)
                    break;
            }
            handle_packet(p, ref, infill, dir);
        }
    }
}
void handle_packet(ETCS_packet *p, distance dist, bool infill, int dir)
{
    if (p->directional) {
        auto *dp = (ETCS_directional_packet*)p;
        if (dir == -1 || (dp->Q_DIR == Q_DIR_t::Nominal && dir == 1) || (dp->Q_DIR == Q_DIR_t::Reverse && dir == 0))
            return;
    }
    if (p->NID_PACKET == 5) {
        Linking l = *(Linking*)p;
        update_linking(dist, l, infill);
    } else if (p->NID_PACKET == 12) {
        Level1_MA ma = *((Level1_MA*)p);
        movement_authority MA = movement_authority(dist, ma);
        distance end = MA.get_end();
        /*if (get_SSP().empty() || (--get_SSP().end())->get_end()<end)
            return;
        if (get_gradient().empty() || (--get_gradient().end())->first<end)
            return;*/
        if (infill)
            MA_infill(MA);
        else
            replace_MA(MA);
    } else if (p->NID_PACKET == 21) {
        GradientProfile grad = *((GradientProfile*)p);
        std::map<distance, double> gradient;
        std::vector<GradientElement> elements;
        elements.push_back(grad.element);
        elements.insert(elements.end(), grad.elements.begin(), grad.elements.end());
        distance d = dist;
        for (auto e : elements) {
            gradient[d+e.D_GRADIENT] = (e.Q_GDIR == Q_GDIR_t::Uphill ? 1 : -1)*e.G_A;
            d += e.D_GRADIENT;
        }
        update_gradient(gradient);
    } else if (p->NID_PACKET == 27) {
        InternationalSSP ISSP = *((InternationalSSP*)p);
        update_SSP(get_SSP(dist, ISSP));
    }
}
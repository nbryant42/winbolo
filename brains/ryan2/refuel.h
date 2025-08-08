#ifndef REFUEL_H
#define REFUEL_H


int getpriorityRefuel(const BrainInfo *info, int idnum);
Boolean startRefuel(const BrainInfo *info, int idnum);
Boolean middleRefuel(const BrainInfo *info, int idnum);
Boolean endRefuel(const BrainInfo *info, int idnum);
Boolean addRefuel(const BrainInfo *info, int idnum);
Boolean isRefuelQueued(const BrainInfo *info, int idnum);








#endif


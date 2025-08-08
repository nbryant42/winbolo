#ifndef ATTACKTANK_H
#define ATTACKTANK_H





Boolean addAttackTank(const BrainInfo *info, int idnum);
Boolean stillChecks(const BrainInfo *info, int idnum);
Boolean startAttackTank(const BrainInfo *info, int idnum);
Boolean middleAttackTank(const BrainInfo *info, int idnum);
Boolean endAttackTank(const BrainInfo *info, int idnum);
int getpriorityAttackTank(const BrainInfo *info, int idnum);


#endif // ATTACKTANK_H




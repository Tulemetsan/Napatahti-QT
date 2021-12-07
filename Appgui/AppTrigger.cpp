#include "AppTrigger.h"

namespace napatahti {

AppTrigger::AppTrigger()
{
    groupList_ = {
        {key(&modeJyotisa), key(&modeWestern)},
        {key(&accMid), key(&accMax), key(&accMin), key(&accAny)},
        {key(&zeroAngAsc), key(&zeroAngAri)},
        {key(&modePrime), key(&modeAsha), key(&modeCosmic)},
        {key(&primeCosDet), key(&primeHorDet), key(&primeNormal)},
        {key(&ashaPowKind), key(&ashaPowCreat), key(&ashaKindDet)},
        {key(&cosmicSum), key(&cosmicDet)},
        {key(&modeGloba), key(&modeSchit)}
    };
}


auto AppTrigger::set(const QAction* action) -> bool
{
    auto index (action->data().toInt());
    auto item (get(index));

    if (item < excBeg())
    {
        *item = action->isChecked();
        return true;
    }

    if (*item)
        return false;

    for (auto& group : groupList_)
        if (group.contains(index))
        {
            for (auto i : group)
                *get(i) = false;

            return *item ^= true;
        }

    return false;
}

} // namespace napatahti

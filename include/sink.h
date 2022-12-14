#ifndef CEREAL_ADVENTURE_SINK_H
#define CEREAL_ADVENTURE_SINK_H

#include "game_object.h"

namespace c_adv {

    class Sink : public GameObject {
    public:
        Sink();
        ~Sink();

        virtual void initialize();

        virtual void render();

        // Assets ----
    public:
        void getAssets(dbasic::AssetManager *am);

    protected:
        static dbasic::ModelAsset *m_sinkAsset;
    };

} /* namespace c_adv */

#endif /* CEREAL_ADVENTURE_SINK_H */

#include "../include/asset_loader.h"

#include "../include/player.h"

c_adv::AssetLoader::AssetLoader() {
    /* void */
}

c_adv::AssetLoader::~AssetLoader() {
    /* void */
}

void c_adv::AssetLoader::createAllMaterials(dbasic::AssetManager *am) {
    dbasic::Material *lightFill = am->NewMaterial();
    lightFill->SetName("LightFill");
    lightFill->SetDiffuseColor(ysVector4(0xEF / 255.0f, 0x38 / 255.0f, 0x37 / 255.0f, 1.0f));

    dbasic::Material *lineColor = am->NewMaterial();
    lineColor->SetName("LineColor");
    lineColor->SetDiffuseColor(ysVector4(0x91 / 255.0f, 0x1A / 255.0f, 0x1D / 255.0f, 1.0f));

    dbasic::Material *darkFill = am->NewMaterial();
    darkFill->SetName("DarkFill");
    darkFill->SetDiffuseColor(ysVector4(0xC4 / 255.0f, 0x21 / 255.0f, 0x26 / 255.0f, 1.0f));

    dbasic::Material *highlight = am->NewMaterial();
    highlight->SetName("Highlight");
    highlight->SetDiffuseColor(ysVector4(1.0f, 1.0f, 1.0f, 1.0f - 0.941667f));

    dbasic::Material *playerMaterial = am->NewMaterial();
    playerMaterial->SetName("PlayerMaterial");
    playerMaterial->SetDiffuseColor(ysVector4(1.0f, 1.0f, 1.0f, 1.0f));
}

void c_adv::AssetLoader::loadAllTextures(const dbasic::Path &assetPath, dbasic::AssetManager *am) {
    /* Load all textures here */
}

void c_adv::AssetLoader::loadAllAssets(const dbasic::Path &assetPath, dbasic::AssetManager *am) {
    loadAllTextures(assetPath, am);
    createAllMaterials(am);

    /* Load all model and animation assets here */
    am->CompileInterchangeFile(getPath("cereal-box/cereal_box", assetPath).c_str(), 1.0f, true);
    am->LoadSceneFile(getPath("cereal-box/cereal_box", assetPath).c_str());

    am->LoadAnimationFile(getPath("cereal-box/cereal_box.dimo", assetPath).c_str());

    am->ResolveNodeHierarchy();

    /* Connect all assets to classes */
    Player::configureAssets(am);
}

std::string c_adv::AssetLoader::getPath(const char *path, const dbasic::Path &assetPath) {
    return assetPath.Append(dbasic::Path(path)).ToString();
}
glslc -c GBuffer.frag
glslc -c GBuffer.vert
glslc -c Composition.frag
glslc -c Composition.vert

cd texture
glslc -c LatlongToCube.frag
glslc -c LatlongToCube.vert
glslc -c IrradianceIBL.frag
glslc -c IrradianceIBL.vert
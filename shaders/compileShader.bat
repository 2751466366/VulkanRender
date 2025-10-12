glslc -c GBuffer.frag
glslc -c GBuffer.vert
glslc -c Composition.frag
glslc -c Composition.vert
glslc -c Lighting.frag
glslc -c Lighting.vert
glslc -c DrawLine.frag
glslc -c DrawLine.vert

cd texture
glslc -c LatlongToCube.frag
glslc -c LatlongToCube.vert
glslc -c IrradianceIBL.frag
glslc -c IrradianceIBL.vert
glslc -c PrefilterIBL.frag
glslc -c PrefilterIBL.vert
glslc -c IntegrateBRDF.frag
glslc -c IntegrateBRDF.vert
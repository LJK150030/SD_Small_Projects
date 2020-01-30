#define MAX_LIGHTS (8)
#define GAMMA (2.2f)
#define HEIGHT_SCALE (1.0f)

struct vs_input_t
{
   float3 position         	: POSITION;
   float4 color            	: COLOR;
   float2 uv               	: UV; 
   float3 normal           	: NORMAL;   // this is new - need buffer layouts to get it there; 
   float4 tangent			: TANGENT;
};


struct light_t 
{
   float3 color; 
   float intensity; 

   float3 position; 
   float is_directional;

   float3 direction; 
   float pad10; 

   float3 diffuse_attenuation; 
   float pad20; 

   float3 specular_attenuation;
   float pad30; 
};

struct lighting_t
{
   float3 diffuse; 
   float3 specular; 
};

//--------------------------------------------------------------------------------------
cbuffer camera_constants : register(b2)
{
   float4x4 VIEW;
   float4x4 PROJECTION;
   float4x4 VIEW_PROJECTION;  // optional 

   float3 CAMERA_POSITION;    
   float cam_unused0;   // NEW - make sure this is updated and passed in per frame; 
};

//--------------------------------------------------------------------------------------
cbuffer model_constants : register(b3)
{
   float4x4 MODEL;  // LOCAL_TO_WORLD
};

//--------------------------------------------------------------------------------------
cbuffer light_buffer : register(b4) 
{
   float4 AMBIENT; 

   float SPEC_FACTOR;   // (0, 1), limits specular amount
   float SPEC_POWER; 
   float EMISSIVE_FACTOR;
   float pad00; 

   light_t LIGHTS[MAX_LIGHTS]; 
};

//--------------------------------------------------------------------------------------
// Texures & Samplers
// ------
// Another option for external data is a Texture.  This is usually a large
// set of data (like an image) that we want to "sample" from.  
//
// A sampler are the rules for how to collect texel data for a given UV. 
//
// Like constant buffers, these hav ea slot they're expecting to be bound
// t0 means use texture unit 0,
// s0 means use sampler unit 0,
//
// In D3D11, constant buffers, textures, and samplers all have their own set 
// of slots.  Some data types may share a slot space (for example, unordered access 
// views (uav) use the texture space). 
//--------------------------------------------------------------------------------------
Texture2D<float4> tAlbedo : register(t0); // texutre I'm using for albedo (color) information
SamplerState sAlbedo : register(s0);      // sampler I'm using for the Albedo texture

Texture2D<float4> tNormal : register(t1);
SamplerState sNormal : register(s1);

Texture2D<float4> tEmissive : register(t2);
SamplerState sEmissive : register(s2);

Texture2D<float4> tSpecular: register(t3);
SamplerState sSpecular : register(s3);

Texture2D<float4> tAmbient : register(t4);
SamplerState sAmbient : register(s4);

Texture2D<float4> tBump : register(t5);
SamplerState sBump: register(s5);
//--------------------------------------------------------------------------------------
lighting_t GetLighting( float3 eye_pos, 
	float3 surface_position, 
	float3 surface_normal,
	float3 surface_specular,
	float3 surface_ambient
)
{
   // 
   lighting_t lighting; 
   lighting.diffuse = (AMBIENT.xyz * AMBIENT.w); 
   //lighting.specular = float3(0.0f,0.0f,0.0f);

   float3 dir_to_eye = normalize(eye_pos - surface_position); 

   for (int i = 0; i < MAX_LIGHTS; ++i) {
	   light_t light = LIGHTS[i]; 

      // directional 
      float3 dir_dir = light.direction; 
      float3 point_dir = normalize(surface_position - light.position); 
      float3 light_dir = lerp( point_dir, dir_dir, light.is_directional ); 

      // common things
      // directional light
      float dir_dist = abs( dot( (surface_position - light.position), light.direction ) );   // for directional
      float point_dist = length( surface_position - light.position );                          // for point
      float distance = lerp( point_dist, dir_dist, light.is_directional ); 

      // Diffuse Part
      float3 la = light.diffuse_attenuation; 
      float attenuation = 1.0f / (la.x + la.y * distance + la.z * distance * distance); 
      float dot3 = max( dot( -light_dir, surface_normal ), 0.0f ); 

      float3 diffuse_color = light.color * light.intensity * attenuation * dot3 * surface_ambient; 
      lighting.diffuse += diffuse_color; 

      // Specular 
      // blinn-phong 
      // dot( H, N );  -> H == half_vector, N == normal
      float3 dir_to_light = -light_dir; 
      float3 half_vector = normalize( dir_to_eye + dir_to_light ); 
      float spec_coefficient = max( dot( half_vector, surface_normal ), 0.0f ); // DO not saturate - spec can go higher;  

      float3 sa = light.specular_attenuation; 
      float spec_attenuation = 1.0f / (sa.x + sa.y * distance + sa.z * distance * distance); 

      // finalize coefficient
      spec_coefficient = SPEC_FACTOR * pow( spec_coefficient, SPEC_POWER ); 
      float3 specular_color = light.color * light.intensity * spec_attenuation * spec_coefficient * surface_specular; 
      lighting.specular += specular_color; 
   }

   lighting.diffuse = saturate(lighting.diffuse); // clamp this to (0, 1)
   //light.specular is untouched - greater than one can tell us information on how bright it is - used for bloom;  

   return lighting; 
}

float2 ParallaxMapping(float2 tex_coords, float3 view_dir, float height)
{   
    float2 new_position = view_dir.xy / view_dir.z * (height * HEIGHT_SCALE);
    return tex_coords - new_position;    
}

//--------------------------------------------------------------------------------------
// for passing data from vertex to fragment (v-2-f)
struct v2f_t 
{
   float4 position : 	SV_POSITION; 
   float4 color : 		COLOR; 
   float2 uv : 			UV; 
   float4 normal:		NORMAL;
   float4 tangent:		TANGENT;
   float4 bitangent:	BITANGENT;
   float4 world_position :	WORLD_POSITION;
}; 

//--------------------------------------------------------------------------------------
float RangeMap( float v, float inMin, float inMax, float outMin, float outMax ) 
{ 
   return ((v - inMin) * ( outMax - outMin)/(inMax - inMin)) + outMin;
}

//--------------------------------------------------------------------------------------
// Vertex Shader
v2f_t VertexFunction(vs_input_t input)
{
   v2f_t v2f = (v2f_t)0;

   float4 local_pos = float4( input.position, 1.0f ); 
   float4 world_pos = mul( MODEL, local_pos ); // mul( MODEL, local_pos ); 
   float4 view_pos = mul( VIEW, world_pos ); 
   float4 clip_pos = mul( PROJECTION, view_pos ); 

   v2f.position = clip_pos; 
   v2f.world_position = world_pos;
   
   float4 local_norm = float4( input.normal, 0.0f);
   float4 world_norm = mul( MODEL, local_norm );
   world_norm = normalize(world_norm);
   v2f.normal = world_norm;
   
   float4 local_tangent = float4( input.tangent.xyz, 0.0f);
   float4 world_tangent = mul( MODEL, local_tangent );
   world_tangent = normalize(world_tangent);
   v2f.tangent = world_tangent;
   
   float3 world_bitangent = cross(world_tangent.xyz, world_norm.xyz) * input.tangent.w;
						
	v2f.color = input.color;
	v2f.uv = input.uv; 
	v2f.normal = world_norm;
	v2f.tangent = world_tangent;
	v2f.bitangent = float4(world_bitangent, 0.0f);
	return v2f;
}

//--------------------------------------------------------------------------------------
float4 FragmentFunction( v2f_t input ) : SV_Target0
{
	//calculating new  uv text coords
	float3x3 tbn = float3x3(input.tangent.xyz, input.bitangent.xyz, input.normal.xyz);
	float3 cam_pos = float3(CAMERA_POSITION.xyz);
	float3 tangent_view_pos = mul(cam_pos, tbn);
	float3 tangent_frag_pos = mul(input.world_position.xyz, tbn);
	float3 view_dir = normalize(tangent_view_pos - tangent_frag_pos);
	float4 surface_height = tBump.Sample(sBump, input.uv);
	float height =  surface_height.x; // can use any color since we are using black to white in the height map   
	float2 tex_coord = ParallaxMapping(input.uv, view_dir, (1.0f - height)); // 1.0 - because we are using a height map and not depth map
	if(tex_coord.x > 1.0 || tex_coord.y > 1.0 || tex_coord.x < 0.0 || tex_coord.y < 0.0)
		discard;
		
	float4 surface_ambient = tAmbient.Sample( sAmbient, tex_coord );
	float4 surface_specular = tSpecular.Sample( sSpecular, tex_coord );
	float4 surface_color = tAlbedo.Sample( sAlbedo, tex_coord ); // from texture; 
	float4 surface_emisive = tEmissive.Sample( sEmissive, tex_coord ) * EMISSIVE_FACTOR;
	float4 normal_color = tNormal.Sample( sNormal, tex_coord);
	
	surface_color = pow( abs(surface_color), GAMMA );  // move to linear space; 
	float3 surface_normal = normal_color.xyz * float3(2.0f, 2.0f, 1.0f) - float3(1.0f, 1.0f, 0.0f); 
	float3 world_normal = mul(surface_normal.xyz, tbn);
	
	
	float3 surface_pos = float3(input.world_position.xyz);

	// lighting for the sceen, any textcoord offsetting ought to be before this
	lighting_t lighting = GetLighting( cam_pos, surface_pos, world_normal, float3(surface_ambient.xyz), float3(surface_specular.xyz) );
	
	float4 final_color = float4(lighting.diffuse, 1.0f) * surface_color + float4(lighting.specular, 0.0f);
	final_color += float4(surface_emisive.xyz * surface_emisive.w, 0);

   final_color = pow( abs(final_color), 1.0f / GAMMA ); // convert back to sRGB space
   return final_color; 
}

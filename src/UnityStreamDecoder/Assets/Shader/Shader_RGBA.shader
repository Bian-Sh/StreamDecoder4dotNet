Shader "Custom/Shader_RGBA" {
	Properties{
		_Color("Color", Color) = (1,1,1,1)
		_RawImg("RawImg", 2D) = "white" {}
	}
		SubShader{
			Tags {"Queue" = "Transparent" "IgnoreProjector" = "True" "RenderType" = "Transparent"}
			Pass{
				Tags { "LightMode" = "ForwardBase" }
				//打开深度写入
				ZWrite On
		//设置为正常透明度混合
		Blend SrcAlpha OneMinusSrcAlpha
		CGPROGRAM

		#pragma vertex vert
		#pragma fragment frag
		#include "Lighting.cginc"

		// Use shader model 3.0 target, to get nicer looking lighting
		#pragma target 3.0

		fixed4 _Color;
		sampler2D _RawImg;
		float4 _RawImg_ST;


		struct a2v {
			float4 vertex : POSITION;
			float4 texcoord : TEXCOORD0;
		};

		struct v2f {
			float4 pos : SV_POSITION;
			float2 uv : TEXCOORD2;
		};

		v2f vert(a2v v) {
			v2f o;
			o.pos = UnityObjectToClipPos(v.vertex);
			//计算uv坐标和偏移量
			o.uv = v.texcoord.xy * _RawImg_ST.xy + _RawImg_ST.zw;
			//竖直反转图像
			o.uv.y = 1 - o.uv.y;
			return o;
		}

		fixed4 frag(v2f i) : SV_Target{
			fixed3 rgb = tex2D(_RawImg, i.uv).rgb;

			return fixed4(rgb * _Color.rgb,_Color.a);
		}

		ENDCG
	}

	}
		FallBack "Diffuse"
}

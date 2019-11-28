Shader "Custom/Shader_I420" {
	Properties{
		_Color("Color", Color) = (1,1,1,1)
		_YTex("YTex", 2D) = "white" {}
		_UTex("UTex", 2D) = "white" {}
		_VTex("VTex", 2D) = "white" {}
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
		sampler2D _YTex;
		float4 _YTex_ST;

		sampler2D _UTex;
		float4 _UTex_ST;

		sampler2D _VTex;
		float4 _VTex_ST;


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
			o.uv = v.texcoord.xy * _YTex_ST.xy + _YTex_ST.zw;
			//竖直反转图像
			o.uv.y = 1 - o.uv.y;
			return o;
		}

		fixed4 frag(v2f i) : SV_Target{
			fixed3 yuv;
			yuv.r = tex2D(_YTex, i.uv).r;
			yuv.g = tex2D(_UTex, i.uv).r - 0.5;
			yuv.b = tex2D(_VTex, i.uv).r - 0.5;

			fixed3 rgb;
			rgb.r = yuv.r + 1.402 * yuv.b;
			rgb.g = yuv.r - 0.34114 * yuv.g - 0.71414 * yuv.b;
			rgb.b = yuv.r + 1.772 * yuv.g;

			return fixed4(rgb * _Color.rgb,_Color.a);
		}

		ENDCG
	}

	}
		FallBack "Diffuse"
}

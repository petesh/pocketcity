/*! \brief Dirt Zone */
#define	Z_DIRT	((welem_t)(0))
/* image:bw/dirt.png */
/*! \brief Real Tree Zone */
#define	Z_REALTREE	((welem_t)(1))
/* image:bw/tree.png */
/*! \brief Real Water Zone */
#define	Z_REALWATER	((welem_t)(2))
/* image:bw/rwater.png */
/*! \brief Fake Tree Zone */
#define	Z_FAKETREE	((welem_t)(3))
/* image:bw/tree.png */
/*! \brief Fake water */
#define	Z_FAKEWATER	((welem_t)(4))
/* image:bw/water.png */
/*! \brief Water pump zone (needs power) */
#define	Z_PUMP	((welem_t)(5))
/* image:bw/pump.png */
/*! \brief wasteland and fire zones */
#define	Z_WASTE	((welem_t)(6))
#define	Z_FIRE1	((welem_t)(7))
#define	Z_FIRE2	((welem_t)(8))
#define	Z_FIRE3	((welem_t)(9))
/* image:bw/wastefire.png */
/*! \brief Crater, caused by meteor */
#define	Z_CRATER	((welem_t)(10))
/* image:bw/crater.png */
/*! \brief water pipe zone and tiles */
#define	Z_PIPE_START	((welem_t)(11))
#define	Z_PIPE	((welem_t)(11))
#define	Z_PIPE_END	((welem_t)(21))
/* image:bw/pipes.png */
/*! \brief power line zone and tiles */
#define	Z_POWERLINE_START	((welem_t)(22))
#define	Z_POWERLINE	((welem_t)(22))
#define	Z_POWERLINE_END	((welem_t)(32))
/* image:bw/powerlines.png */
/*! \brief Power and water overlap */
#define	Z_POWERWATER_START	((welem_t)(33))
#define	Z_POWER_WATER_PHOR	((welem_t)(33))
#define	Z_POWERWATER_END	((welem_t)(34))
#define	Z_POWER_WATER_PVER	((welem_t)(34))
/* image:bw/powerwateroverlap.png */
/*! \brief Slum zones - only zoned */
#define	Z_COMMERCIAL_SLUM	((welem_t)(35))
#define	Z_RESIDENTIAL_SLUM	((welem_t)(36))
#define	Z_INDUSTRIAL_SLUM	((welem_t)(37))
/* image:bw/slums.png */
/*! \brief Coal power plant */
#define	Z_COALPLANT	((welem_t)(38))
#define	Z_COALPLANT_START	((welem_t)(38))
#define	Z_COALPLANT_END	((welem_t)(41))
/* image:bw/coalplant.png */
/*! \brief Nuclear power plant */
#define	Z_NUCLEARPLANT	((welem_t)(42))
#define	Z_NUCLEARPLANT_START	((welem_t)(42))
#define	Z_NUCLEARPLANT_END	((welem_t)(45))
/* image:bw/nuclearplant.png */
/*! \brief Fire station */
#define	Z_FIRESTATION	((welem_t)(46))
#define	Z_FIRESTATION_START	((welem_t)(46))
#define	Z_FIRESTATION_END	((welem_t)(49))
/* image:bw/firestation.png */
/*! \brief Police Department */
#define	Z_POLICEDEPT	((welem_t)(50))
#define	Z_POLICEDEPT_START	((welem_t)(50))
#define	Z_POLICEDEPT_END	((welem_t)(53))
/* image:bw/policedept.png */
/*! \brief Army base */
#define	Z_ARMYBASE	((welem_t)(54))
#define	Z_ARMYBASE_START	((welem_t)(54))
#define	Z_ARMYBASE_END	((welem_t)(57))
/* image:bw/armybase.png */
/*! \brief Commercial zones */
#define	Z_COMMERCIAL_MIN	((welem_t)(58))
#define	Z_COMMERCIAL_MAX	((welem_t)(67))
/* image:bw/commercialzones.png */
/*! \brief Residential zones */
#define	Z_RESIDENTIAL_MIN	((welem_t)(68))
#define	Z_RESIDENTIAL_MAX	((welem_t)(77))
/* image:bw/residentialzones.png */
/*! \brief Industrial zones */
#define	Z_INDUSTRIAL_MIN	((welem_t)(78))
#define	Z_INDUSTRIAL_MAX	((welem_t)(87))
/* image:bw/industrialzones.png */
/*! \brief Power overlapping road */
#define	Z_POWERROAD_START	((welem_t)(88))
#define	Z_POWERROAD_PHOR	((welem_t)(88))
#define	Z_POWERROAD_END	((welem_t)(89))
#define	Z_POWERROAD_PVER	((welem_t)(89))
/* image:bw/powerroadoverlap.png */
/*! \brief Pipe overlapping road */
#define	Z_PIPEROAD_PHOR	((welem_t)(90))
#define	Z_PIPEROAD_START	((welem_t)(90))
#define	Z_PIPEROAD_PVER	((welem_t)(91))
#define	Z_PIPEROAD_END	((welem_t)(91))
/* image:bw/piperoadoverlap.png */
/*! \brief Road zone and tiles */
#define	Z_ROAD_START	((welem_t)(92))
#define	Z_ROAD	((welem_t)(92))
#define	Z_ROAD_END	((welem_t)(102))
/* image:bw/road.png */
/*! \brief Bridge zones */
#define	Z_BRIDGE	((welem_t)(103))
#define	Z_BRIDGE_START	((welem_t)(103))
#define	Z_BRIDGE_HOR	((welem_t)(103))
#define	Z_BRIDGE_END	((welem_t)(104))
#define	Z_BRIDGE_VER	((welem_t)(104))
/* image:bw/bridge.png */
/*! \brief Rail zone and tiles */
#define	Z_RAIL_START	((welem_t)(105))
#define	Z_RAIL	((welem_t)(105))
#define	Z_RAIL_END	((welem_t)(115))
/* image:bw/rail.png */
/*! \brief Rail overlapping water pipe */
#define	Z_RAILPIPE_START	((welem_t)(116))
#define	Z_RAILPIPE_RHOR	((welem_t)(116))
#define	Z_RAILPIPE_END	((welem_t)(117))
#define	Z_RAILPIPE_RVER	((welem_t)(117))
/* image:bw/railpipe.png */
/*! \brief Rail overlapping power line */
#define	Z_RAILPOWER_START	((welem_t)(118))
#define	Z_RAILPOWER_RHOR	((welem_t)(118))
#define	Z_RAILPOWER_END	((welem_t)(119))
#define	Z_RAILPOWER_RVER	((welem_t)(119))
/* image:bw/railpower.png */
/*! \brief Rail overlapping road */
#define	Z_RAILOVROAD_START	((welem_t)(120))
#define	Z_RAILOVROAD_RHOR	((welem_t)(120))
#define	Z_RAILOVROAD_END	((welem_t)(121))
#define	Z_RAILOVROAD_RVER	((welem_t)(121))
/* image:bw/railovroad.png */
/*! \brief Rail tunnel */
#define	Z_RAILTUNNEL_START	((welem_t)(122))
#define	Z_RAILTUNNEL_RHOR	((welem_t)(122))
#define	Z_RAILTUNNEL_END	((welem_t)(123))
#define	Z_RAILTUNNEL_RVER	((welem_t)(123))
#define	Z_ENDMARKER	((welem_t)(123))
/* image:bw/railtunnel.png */
/*! \brief Status flags for rendered zones */
#define	Z_POWER_OUT	((welem_t)(124))
#define	Z_POWER_OUT_MASK	((welem_t)(125))
#define	Z_WATER_OUT	((welem_t)(126))
#define	Z_WATER_OUT_MASK	((welem_t)(127))
/* image:bw/statusimages.png */

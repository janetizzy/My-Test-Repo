#!/usr/bin/env lua

--
--	tsp.lua - Travelling salesman solver
--	Written by Timm S. Mueller <tmueller at schulze-mueller.de>
--	See copyright notice in COPYRIGHT
--
--	Demonstrates a computation-heavy program with a reactive GUI
--

ui = require "tek.ui"
local Frame = ui.Frame
local floor = math.floor
local sqrt = math.sqrt
local random = math.random

DEFAULT_DECAY = 1.7
REFRESH_DELAY = 15

-------------------------------------------------------------------------------
--	coordinates of 980 municipals in Luxembourg:
-------------------------------------------------------------------------------

local luxembourg =
{
	{0.256,0.910},{0.256,0.910},{0.780,0.614},{0.858,0.795},{0.534,0.534},{0.028,0.490},{0.790,0.588},{0.967,0.645},{0.743,0.899},{0.967,0.645},
	{0.635,0.784},{0.743,0.899},{0.790,0.944},{0.228,0.178},{0.191,0.297},{0.228,0.178},{0.510,0.774},{0.398,0.738},{0.245,0.295},{0.332,0.267},
	{0.332,0.267},{0.364,0.057},{0.398,0.738},{0.245,0.295},{0.535,0.337},{0.484,0.460},{0.361,0.888},{0.592,0.599},{0.592,0.641},{0.170,0.930},
	{0.228,0.178},{0.402,0.036},{0.402,0.036},{0.447,0.506},{0.421,0.407},{0.776,0.913},{0.701,0.893},{0.430,0.062},{0.484,0.853},{0.510,0.774},
	{0.527,0.807},{0.613,0.437},{0.613,0.437},{0.494,0.303},{0.593,0.889},{0.828,0.483},{0.263,0.107},{0.494,0.303},{0.258,0.290},{0.319,0.199},
	{0.836,0.680},{0.657,0.506},{0.657,0.506},{0.510,0.774},{0.056,0.329},{0.981,0.654},{0.456,0.454},{0.160,0.529},{0.056,0.329},{0.511,0.726},
	{0.558,0.354},{0.705,0.869},{0.704,0.545},{0.721,0.506},{0.497,0.272},{0.497,0.272},{0.212,0.479},{0.333,0.525},{0.603,0.342},{0.170,0.930},
	{0.310,0.571},{0.393,0.147},{0.521,0.250},{0.333,0.084},{0.693,0.767},{0.346,0.644},{0.346,0.644},{0.182,0.163},{0.333,0.084},{0.175,0.188},
	{0.182,0.163},{0.696,0.847},{0.153,0.307},{0.052,0.368},{0.052,0.368},{0.545,0.398},{0.894,0.668},{0.388,0.979},{0.069,0.321},{0.570,0.394},
	{0.327,0.375},{0.620,0.860},{0.048,0.387},{0.048,0.387},{0.428,0.778},{0.550,0.297},{0.550,0.297},{0.279,0.143},{0.493,0.700},{0.230,0.674},
	{0.952,0.514},{0.228,0.722},{0.613,0.437},{0.371,0.895},{0.556,0.488},{0.160,0.506},{0.805,0.859},{0.290,0.833},{0.109,0.301},{0.109,0.301},
	{0.256,0.910},{0.291,0.667},{0.468,0.768},{0.291,0.667},{0.367,0.237},{0.291,0.667},{0.468,0.768},{0.643,0.749},{0.267,0.227},{0.575,0.456},
	{0.506,0.645},{0.145,0.487},{0.145,0.487},{0.239,0.637},{0.795,0.929},{0.795,0.929},{0.297,0.604},{0.613,0.645},{0.681,0.778},{0.539,0.533},
	{0.668,0.799},{0.277,0.831},{0.554,0.607},{0.452,0.642},{0.268,0.691},{0.366,0.000},{0.415,0.345},{0.149,0.265},{0.409,0.837},{0.409,0.837},
	{0.149,0.265},{0.506,0.645},{0.142,0.238},{0.350,0.937},{0.461,0.487},{0.742,0.821},{0.742,0.821},{0.807,0.985},{0.800,0.564},{0.391,0.400},
	{0.302,0.540},{0.662,0.524},{0.241,0.879},{0.330,0.114},{0.241,0.879},{0.616,0.788},{0.165,0.675},{0.315,0.630},{0.165,0.675},{0.301,0.141},
	{0.376,1.000},{0.376,1.000},{0.236,0.313},{0.457,0.548},{0.986,0.533},{0.457,0.548},{0.725,0.720},{0.336,0.679},{0.102,0.537},{0.102,0.537},
	{0.725,0.720},{0.040,0.437},{0.040,0.437},{0.000,0.481},{0.472,0.775},{0.481,0.610},{0.744,0.878},{0.147,0.881},{0.744,0.878},{0.527,0.887},
	{0.702,0.852},{0.702,0.852},{0.378,0.842},{0.702,0.852},{0.503,0.201},{0.441,0.175},{0.135,0.903},{0.707,0.692},{0.336,0.679},{0.249,0.558},
	{0.127,0.881},{0.278,0.597},{0.746,0.621},{0.348,0.367},{0.205,0.460},{0.301,0.141},{0.382,0.172},{0.670,0.603},{0.808,0.883},{0.788,0.961},
	{0.680,0.434},{0.273,0.533},{0.240,0.566},{0.192,0.577},{0.192,0.577},{0.192,0.577},{0.153,0.307},{0.743,0.899},{0.344,0.859},{0.440,0.599},
	{0.440,0.599},{0.344,0.859},{0.635,0.728},{0.635,0.728},{0.132,0.487},{0.518,0.306},{0.549,0.784},{0.489,0.644},{0.489,0.644},{0.254,0.527},
	{0.852,0.705},{0.852,0.705},{0.371,0.895},{0.153,0.307},{0.248,0.542},{0.474,0.571},{0.474,0.571},{0.333,0.877},{0.350,0.260},{0.451,0.234},
	{0.333,0.368},{0.333,0.368},{0.506,0.784},{0.333,0.368},{0.474,0.571},{0.086,0.558},{0.178,0.865},{0.178,0.865},{0.030,0.513},{0.501,0.912},
	{0.343,0.103},{0.904,0.559},{0.209,0.552},{0.333,0.728},{0.761,0.712},{0.741,0.701},{0.678,0.784},{0.670,0.812},{0.153,0.229},{0.419,0.321},
	{0.462,0.483},{0.131,0.626},{0.355,0.483},{0.198,0.927},{0.218,0.838},{0.528,0.561},{0.379,0.463},{0.523,0.242},{0.851,0.742},{0.100,0.573},
	{0.305,0.035},{0.645,0.733},{0.181,0.333},{0.344,0.703},{0.559,0.548},{0.149,0.545},{0.406,0.933},{0.218,0.280},{0.237,0.610},{0.306,0.325},
	{0.225,0.666},{0.233,0.295},{0.131,0.236},{0.419,0.345},{0.462,0.483},{0.219,0.592},{0.355,0.483},{0.195,0.896},{0.217,0.852},{0.394,0.452},
	{0.855,0.763},{0.109,0.587},{0.312,0.050},{0.592,0.414},{0.670,0.741},{0.187,0.422},{0.232,0.140},{0.268,0.391},{0.204,0.668},{0.635,0.784},
	{0.247,0.853},{0.644,0.780},{0.536,0.784},{0.510,0.774},{0.419,0.212},{0.394,0.452},{0.149,0.545},{0.501,0.324},{0.679,0.766},{0.383,0.205},
	{0.905,0.677},{0.735,0.551},{0.501,0.696},{0.487,0.763},{0.501,0.696},{0.664,0.827},{0.613,0.622},{0.484,0.668},{0.980,0.615},{0.470,0.600},
	{0.318,0.900},{0.697,0.939},{0.318,0.900},{0.935,0.604},{0.643,0.435},{0.484,0.574},{0.980,0.615},{0.117,0.437},{0.117,0.437},{0.359,0.504},
	{0.359,0.504},{0.359,0.504},{0.452,0.391},{0.577,0.583},{0.343,0.482},{0.343,0.482},{0.956,0.674},{0.471,0.315},{0.297,0.430},{0.471,0.315},
	{0.297,0.430},{0.470,0.600},{0.457,0.802},{0.309,0.304},{0.309,0.304},{0.309,0.304},{0.160,0.506},{0.309,0.304},{0.726,0.736},{0.410,0.681},
	{0.655,0.845},{0.616,0.527},{0.356,0.195},{0.168,0.358},{0.356,0.195},{0.168,0.358},{0.657,0.807},{0.204,0.414},{0.324,0.089},{0.381,0.348},
	{0.604,0.391},{0.040,0.437},{0.471,0.315},{0.413,0.178},{0.413,0.654},{0.430,0.180},{0.885,0.664},{0.363,0.769},{0.160,0.529},{0.790,0.588},
	{0.570,0.645},{0.125,0.544},{0.125,0.544},{0.225,0.666},{0.228,0.515},{0.506,0.622},{0.761,0.712},{0.311,0.668},{0.249,0.558},{0.549,0.923},
	{0.635,0.414},{0.139,0.599},{0.700,0.668},{0.247,0.853},{0.915,0.576},{0.146,0.499},{0.139,0.923},{0.570,0.483},{0.895,0.730},{0.895,0.730},
	{0.501,0.791},{0.501,0.791},{0.192,0.378},{0.258,0.169},{0.140,0.447},{0.140,0.447},{0.517,0.676},{0.517,0.676},{0.599,0.392},{0.489,0.904},
	{0.445,0.365},{0.506,0.646},{0.186,0.854},{0.490,0.778},{0.308,0.864},{0.475,0.079},{0.174,0.377},{0.147,0.613},{0.417,0.851},{0.501,0.791},
	{0.305,0.155},{0.809,0.809},{0.352,0.275},{0.444,0.530},{0.352,0.275},{0.898,0.646},{0.474,0.029},{0.462,0.529},{0.836,0.543},{0.441,0.087},
	{0.123,0.911},{0.619,0.561},{0.120,0.531},{0.125,0.544},{0.502,0.357},{0.490,0.778},{0.147,0.881},{0.147,0.881},{0.139,0.923},{0.172,0.812},
	{0.172,0.812},{0.207,0.432},{0.505,0.541},{0.225,0.666},{0.225,0.666},{0.523,0.896},{0.268,0.710},{0.428,0.718},{0.403,0.287},{0.126,0.477},
	{0.268,0.710},{0.605,0.629},{0.478,0.861},{0.424,0.156},{0.424,0.156},{0.729,0.598},{0.357,0.610},{0.287,0.234},{0.357,0.610},{0.374,0.176},
	{0.793,0.901},{0.292,0.225},{0.225,0.743},{0.526,0.774},{0.508,0.386},{0.748,0.703},{0.423,0.676},{0.381,0.390},{0.379,0.712},{0.384,0.961},
	{0.359,0.318},{0.207,0.361},{0.295,0.613},{0.635,0.437},{0.794,0.759},{0.321,0.745},{0.313,0.745},{0.197,0.560},{0.479,0.109},{0.227,0.763},
	{0.661,0.658},{0.592,0.414},{0.808,0.645},{0.808,0.645},{0.268,0.506},{0.558,0.829},{0.436,0.981},{0.069,0.321},{0.069,0.321},{0.184,0.388},
	{0.507,0.457},{0.589,0.672},{0.835,0.807},{0.202,0.618},{0.410,0.119},{0.503,0.673},{0.417,0.922},{0.352,0.024},{0.366,0.000},{0.352,0.024},
	{0.419,0.067},{0.213,0.637},{0.074,0.321},{0.160,0.516},{0.637,0.736},{0.160,0.516},{0.637,0.736},{0.454,0.227},{0.535,0.337},{0.535,0.337},
	{0.452,0.289},{0.439,0.325},{0.228,0.515},{0.228,0.515},{0.228,0.515},{0.333,0.136},{0.428,0.273},{0.320,0.784},{0.063,0.519},{0.063,0.519},
	{0.494,0.803},{0.397,0.081},{0.399,0.650},{0.439,0.325},{0.619,0.762},{0.234,0.114},{0.667,0.413},{0.247,0.853},{0.619,0.762},{0.223,0.684},
	{0.245,0.797},{1.000,0.562},{0.244,0.428},{0.529,0.851},{0.758,0.598},{0.244,0.428},{0.887,0.610},{0.768,0.620},{0.516,0.734},{0.519,0.686},
	{0.528,0.936},{0.178,0.450},{0.069,0.321},{0.178,0.450},{0.517,0.712},{0.209,0.188},{0.517,0.712},{0.447,0.119},{0.284,0.386},{0.306,0.406},
	{0.645,0.582},{0.247,0.691},{0.419,0.321},{0.355,0.483},{0.011,0.495},{0.379,0.463},{0.100,0.573},{0.218,0.838},{0.305,0.035},{0.575,0.917},
	{0.604,0.878},{0.061,0.347},{0.551,0.797},{0.209,0.188},{0.699,0.512},{0.178,0.450},{0.245,0.738},{0.806,0.685},{0.247,0.853},{0.244,0.428},
	{0.234,0.102},{0.204,0.391},{0.762,0.481},{0.506,0.784},{0.168,0.274},{0.168,0.274},{0.291,0.491},{0.620,0.418},{0.377,0.582},{0.411,0.136},
	{0.852,0.705},{0.905,0.703},{0.377,0.582},{0.377,0.582},{0.592,0.761},{0.592,0.761},{0.224,0.456},{0.224,0.456},{0.453,0.485},{0.798,0.829},
	{0.318,0.655},{0.705,0.620},{0.192,0.764},{0.192,0.764},{0.465,0.339},{0.309,0.723},{0.792,0.780},{0.484,0.632},{0.292,0.361},{0.292,0.361},
	{0.654,0.684},{0.309,0.723},{0.309,0.723},{0.292,0.361},{0.146,0.499},{0.350,0.045},{0.350,0.045},{0.288,0.710},{0.403,0.360},{0.633,0.628},
	{0.350,0.045},{0.954,0.610},{0.954,0.610},{0.979,0.575},{0.980,0.560},{0.979,0.575},{0.569,0.437},{0.835,0.586},{0.333,0.599},{0.492,0.827},
	{0.271,0.784},{0.549,0.807},{0.165,0.675},{0.343,0.103},{0.872,0.714},{0.872,0.714},{0.581,0.925},{0.160,0.506},{0.587,0.369},{0.366,0.000},
	{0.535,0.337},{0.284,0.386},{0.171,0.510},{0.597,0.470},{0.347,0.905},{0.091,0.392},{0.060,0.479},{0.778,0.721},{0.060,0.479},{0.060,0.479},
	{0.060,0.479},{0.422,0.148},{0.422,0.148},{0.575,0.613},{0.165,0.675},{0.386,0.586},{0.205,0.802},{0.595,0.774},{0.654,0.901},{0.394,0.452},
	{0.170,0.930},{0.398,0.738},{0.592,0.599},{0.170,0.930},{0.484,0.506},{0.444,0.530},{0.135,0.903},{0.743,0.899},{0.117,0.437},{0.040,0.437},
	{0.140,0.447},{0.794,0.759},{0.549,0.414},{0.592,0.761},{0.074,0.345},{0.872,0.714},{0.597,0.470},{0.074,0.345},{0.398,0.044},{0.170,0.930},
	{0.705,0.935},{0.613,0.784},{0.135,0.903},{0.532,0.859},{0.451,0.234},{0.434,0.918},{0.619,0.561},{0.613,0.345},{0.275,0.565},{0.468,0.464},
	{0.468,0.464},{0.484,0.559},{0.338,0.165},{0.735,0.652},{0.267,0.255},{0.735,0.652},{0.251,0.375},{0.308,0.951},{0.069,0.321},{0.188,0.497},
	{0.251,0.410},{0.251,0.375},{0.308,0.951},{0.251,0.375},{0.690,0.842},{0.481,0.440},{0.757,0.884},{0.266,0.282},{0.481,0.440},{0.757,0.884},
	{0.266,0.282},{0.620,0.577},{0.653,0.702},{0.260,0.931},{0.621,0.489},{0.656,0.468},{0.324,0.253},{0.835,0.807},{0.277,0.122},{0.277,0.122},
	{0.711,0.964},{0.227,0.634},{0.743,0.938},{0.227,0.634},{0.743,0.938},{0.197,0.560},{0.197,0.560},{0.719,0.918},{0.148,0.580},{0.573,0.687},
	{0.176,0.688},{0.504,0.767},{0.268,0.644},{0.835,0.807},{0.277,0.122},{0.277,0.122},{0.289,0.912},{0.355,0.878},{0.880,0.522},{0.446,0.973},
	{0.446,0.973},{0.358,0.067},{0.358,0.067},{0.358,0.067},{0.843,0.783},{0.341,0.227},{0.436,0.198},{0.436,0.198},{0.425,0.202},{0.276,0.166},
	{0.276,0.166},{0.122,0.294},{0.513,0.766},{0.276,0.166},{0.383,0.361},{0.311,0.823},{0.746,0.455},{0.746,0.455},{0.127,0.632},{0.192,0.912},
	{0.192,0.912},{0.383,0.361},{0.535,0.435},{0.127,0.632},{0.944,0.561},{0.452,0.289},{0.190,0.231},{0.383,0.361},{0.283,0.447},{0.286,0.161},
	{0.277,0.122},{0.705,0.935},{0.669,0.889},{0.266,0.804},{0.302,0.342},{0.505,0.541},{0.201,0.172},{0.523,0.896},{0.626,0.832},{0.403,0.287},
	{0.771,0.567},{0.100,0.573},{0.109,0.587},{0.459,0.515},{0.726,0.582},{0.726,0.582},{0.374,0.176},{0.374,0.176},{0.172,0.812},{0.509,0.791},
	{0.570,0.437},{0.570,0.437},{0.570,0.437},{0.343,0.103},{0.687,0.556},{0.577,0.583},{0.613,0.345},{0.843,0.783},{0.451,0.492},{0.613,0.345},
	{0.471,0.822},{0.321,0.745},{0.313,0.745},{0.755,0.797},{0.287,0.633},{0.953,0.588},{0.250,0.492},{0.344,0.598},{0.420,0.377},{0.751,0.966},
	{0.150,0.390},{0.461,0.414},{0.453,0.958},{0.258,0.344},{0.423,0.956},{0.805,0.726},{0.211,0.277},{0.140,0.447},{0.776,0.635},{0.776,0.635},
	{0.776,0.635},{0.363,0.618},{0.575,0.456},{0.439,0.726},{0.727,0.570},{0.774,0.659},{0.727,0.570},{0.415,0.080},{0.208,0.452},{0.514,0.373},
	{0.397,0.728},{0.218,0.222},{0.323,0.136},{0.353,0.564},{0.353,0.564},{0.506,0.622},{0.759,0.871},{0.953,0.588},{0.420,0.377},{0.617,0.671},
	{0.150,0.390},{0.357,0.666},{0.095,0.408},{0.793,0.644},{0.793,0.644},{0.998,0.593},{0.527,0.815},{0.188,0.386},{0.065,0.458},{0.188,0.386},
	{0.419,0.171},{0.131,0.288},{0.516,0.692},{0.353,0.564},{0.138,0.360},{0.138,0.360},{0.353,0.564},{0.248,0.188},{0.248,0.188},{0.248,0.188},
	{0.330,0.358},{0.401,0.235},{0.401,0.235},{0.401,0.235},{0.330,0.358},{0.679,0.615},{0.554,0.672},{0.306,0.088},{0.818,0.664},{0.582,0.307},
	{0.511,0.884},{0.582,0.307},{0.423,0.547},{0.333,0.136},{0.451,0.492},{0.613,0.784},{0.613,0.784},{0.613,0.784},{0.511,0.884},{0.383,0.087},
	{0.106,0.450},{0.643,0.586},{0.065,0.458},{0.706,0.446},{0.769,0.770},{0.786,0.691},{0.615,0.423},{0.466,0.920},{0.628,0.367},{0.257,0.536},
	{0.312,0.840},{0.312,0.840},{0.398,0.792},{0.482,0.607},{0.147,0.318},{0.484,0.587},{0.798,0.699},{0.388,0.911},{0.448,0.511},{0.160,0.576},
	{0.798,0.699},{0.680,0.469},{0.498,0.728},{0.786,0.511},{0.500,0.890},{0.500,0.890},{0.843,0.630},{0.242,0.928},{0.260,0.931},{0.242,0.928},
	{0.448,0.022},{0.725,0.636},{0.499,0.739},{0.711,0.480},{0.187,0.626},{0.804,0.604},{0.791,0.907},{0.711,0.480},{0.138,0.360},{0.095,0.408},
	{0.547,0.402},{0.355,0.483},{0.109,0.587},{0.100,0.393},{0.204,0.414},{0.217,0.852},{0.312,0.050},{0.175,0.188},{0.761,0.712},{0.139,0.923},
	{0.451,0.234},{0.398,0.391},{0.298,0.117},{0.398,0.391},{0.742,0.872},{0.624,0.914},{0.131,0.444},{0.389,0.665},{0.541,0.590},{0.160,0.576},
	{0.545,0.855},{0.666,0.931},{0.758,0.607},{0.758,0.607},{0.613,0.647},{0.311,0.668},{0.346,0.292},{0.166,0.201},{0.135,0.903},{0.879,0.773},
	{0.204,0.899},{0.438,0.899},{0.436,0.981},{0.170,0.930},{0.198,0.927},{0.135,0.903},{0.195,0.896},{0.344,0.859},{0.312,0.840},{0.409,0.837},
	{0.471,0.822},{0.397,0.728},{0.230,0.674},{0.225,0.666},{0.357,0.610},{0.386,0.586},{0.120,0.531},{0.444,0.530},{0.460,0.967},{0.793,0.903},
	{0.545,0.855},{0.742,0.821},{0.644,0.780},{0.619,0.762},{0.679,0.766},{0.748,0.703},{0.967,0.645},{0.643,0.586},{0.790,0.588},{0.835,0.586},
	{0.735,0.551},{0.534,0.534},{0.836,0.543},{0.000,0.481},{0.507,0.457},{0.140,0.447},{0.040,0.437},{0.091,0.392},{0.150,0.390},{0.284,0.386},
	{0.508,0.386},{0.176,0.359},{0.131,0.288},{0.211,0.277},{0.451,0.234},{0.356,0.195},{0.436,0.198},{0.175,0.188},{0.248,0.188},{0.276,0.166},
	{0.286,0.161},{0.232,0.140},{0.277,0.122},{0.402,0.036},{0.366,0.000},{0.234,0.102},{0.620,0.418},{0.545,0.398},{0.558,0.354},{0.535,0.337},
}

-------------------------------------------------------------------------------
--	TSP solver + visualization class
-------------------------------------------------------------------------------

local TSP = Frame:newClass()

function TSP.new(class, self)
	self = self or { }
	self = Frame.new(class, self)
	self:reset()
	return self
end

function TSP:reset()
	self.Frame = 1
	self.Length = 0
	self.Iterations = 0
	self.Accepted = 0
	self.Bombs = 0
	for i = 1, #self.Map do
		self.Points[i] = self.Map[i]
	end
	self.Length = self:length()
	self.Threshold = self.Length / #self.Points / 3
end

function TSP.getDecay(v)
	return 1 - 1 / math.pow(10, 8 - v)
end

function TSP.init(self)
	self.Decay = 0.9999999
	self.Frame = 1
	self.Length = 0
	self.Threshold = 0
	self.Changed = false
	self.Iterations = 0
	self.Accepted = 0
	self.Bombs = 0
	self.Points = { }
	return Frame.init(self)
end

function TSP:show(drawable)
	Frame.show(self, drawable)
	self.Window:addInputHandler(ui.MSG_INTERVAL, self, self.update)
end

function TSP:hide()
	self.Window:remInputHandler(ui.MSG_INTERVAL, self, self.update)
	Frame.hide(self)
end

function TSP:update()
	if self.Drawable then
		self.Frame = self.Frame + 1
		if self.Changed and (self.Frame % REFRESH_DELAY == 0) then
			self.Changed = false
			self.Flags:set(ui.FL_REDRAW)
		end
	end
end

function TSP:draw()
	if Frame.draw(self) then
		local t = 15 - (1 - math.log10(self.Threshold))
		self:getById("gauge-thresh"):setValue("Value", t)
		self:getById("text-length"):setValue("Text", ("%.4f"):format(self.Length))
		self:getById("text-iter"):setValue("Text", self.Iterations)
		self:getById("text-accept"):setValue("Text", self.Accepted)
		local x0, y0, x1, y1 = self:getRect()
		local w = x1 - x0 + 1 - 10
		local h = y1 - y0 + 1 - 10
		local d = self.Drawable
		local p = self.Points
		for i = 2, #self.Points do
			local x = x0 + p[i - 1][1] * w + 5
			local y = y0 + p[i - 1][2] * h + 5
			local x1 = x0 + p[i][1] * w + 5
			local y1 = y0 + p[i][2] * h + 5
			d:drawLine(x, y, x1, y1, d.Pens["shine"])
		end
	end
end

function TSP:length()
	local p = self.Points
	local tl = 0
	for i = 2, #p do
		local x0 = p[i - 1][1]
		local y0 = p[i - 1][2]
		local x1 = p[i][1]
		local y1 = p[i][2]
		local dx = x1 - x0
		local dy = y1 - y0
		local l = sqrt(dx * dx + dy * dy)
		tl = tl + l
	end
	return tl
end

function TSP:opt()
	
	local i1, i2
	local p = self.Points
	
	for i = 1, 1000 do
		self.Iterations = self.Iterations + 1
	
		repeat
			i1 = floor(random() * #p) + 1
			i2 = floor(random() * #p) + 1
			if i2 < i1 then
				i1, i2 = i2, i1
			end
		until i2 ~= i1 and i1 ~= 1 and i2 ~= #p
		
		local x0, y0, x1, y1, x2, y2, x3, y3
		local dx, dy
		
		local dtl
		
		x0 = p[i1 - 1][1]
		y0 = p[i1 - 1][2]
		x1 = p[i1][1]
		y1 = p[i1][2]
		x2 = p[i2][1]
		y2 = p[i2][2]
		x3 = p[i2 + 1][1]
		y3 = p[i2 + 1][2]
		
		dx = x1 - x0
		dy = y1 - y0
		dtl = sqrt(dx * dx + dy * dy)
		
		dx = x3 - x2
		dy = y3 - y2
		dtl = dtl + sqrt(dx * dx + dy * dy)
	
		dx = x2 - x0
		dy = y2 - y0
		dtl = -dtl + sqrt(dx * dx + dy * dy)
	
		dx = x3 - x1
		dy = y3 - y1
		dtl = dtl + sqrt(dx * dx + dy * dy)
		
		-- threshold accepting:
		local accept = dtl < self.Threshold
		if not accept then
			-- drop some bombs:
			if dtl < self.Length / #p and random() < 0.001 then
				accept = true
				self.Bombs = self.Bombs + 1
			end
		end
		
		if accept then
			local i3 = i1 + floor((i2 - i1 - 1) / 2)
			local j = i2
			for i = i1, i3 do
				p[i], p[j] = p[j], p[i]
				j = j - 1
			end
			self.Length = self.Length + dtl
			self.Changed = true
			self.Accepted = self.Accepted + 1
		end
		
		self.Threshold = self.Threshold * self.Decay
	
	end
	
	-- refresh length, so that floating point inaccuracies
	-- remain under control:
	self.Length = self:length()
	
end

-------------------------------------------------------------------------------
--	main program:
-------------------------------------------------------------------------------

math.randomseed(os.time())

app = ui.Application:new
{
	Children =
	{
		ui.Window:new
		{
			Width = 400, Height = 600,
			MaxWidth = "none", MaxHeight = "none",
			MinWidth = 0, MinHeight = 0,
			Title = "Travelling Salesman Approximation",
			Orientation = "vertical",
			HideOnEscape = true,
			Children =
			{
				ui.Group:new
				{
					Columns = 2,
					Children =
					{
						ui.Text:new
						{
							Class = "caption",
							Text = "Path Length:",
							Style = "text-align: right",
							Width = "fill",
						},
						ui.Text:new
						{
							Id = "text-length",
							Style = "text-align: left",
						},
						ui.Text:new
						{
							Class = "caption",
							Text = "Iterations:",
							Style = "text-align: right",
							Width = "fill",
						},
						ui.Text:new
						{
							Id = "text-iter",
							Style = "text-align: left",
						},
						ui.Text:new
						{
							Class = "caption",
							Text = "Optimizations:",
							Style = "text-align: right",
							Width = "fill",
						},
						ui.Text:new
						{
							Id = "text-accept",
							Style = "text-align: left",
						},
						ui.Text:new
						{
							Class = "caption",
							Text = "Decay:",
							Style = "text-align: right",
							Width = "fill",
						},
						ui.Group:new
						{
							Children =
							{
								ui.ScrollBar:new
								{
									Min = 1,
									Max = 4,
									Value = DEFAULT_DECAY,
									Kind = "number",
									Width = "free",
									onSetValue = function(self, value)
										ui.ScrollBar.onSetValue(self, value)
										local t = ("%.2f"):format(self.Value)
										self:getById("decay-value"):setValue("Text", t)
										self:getById("the-tsp").Decay = TSP.getDecay(self.Value)
									end,
									show = function(self, display, drawable)
										self:setValue("Value", self.Value, true)
										return ui.ScrollBar.show(self, display, drawable)
									end
								},
								ui.Text:new
								{
									MaxWidth = 0,
									Id = "decay-value",
									Font = "ui-fixed",
									Text = "1.00",
								}
							}
						},
						ui.Text:new
						{
							Mode = "button",
							Class = "button",
							Text = "Restart",
							MaxWidth = 0,
							onPress = function(self, pressed)
								ui.Text.onPress(self, pressed)
								if pressed == false then
									self:getById("the-tsp"):reset()
								end
							end
						},
					}
				},
				ui.Group:new
				{
					Children =
					{
						ui.Gauge:new
						{
							Min = 2,
							Max = 15,
							Id = "gauge-thresh",
							Orientation = "vertical",
							MaxWidth = 0,
							Height = "free",
						},
						TSP:new 
						{
							MinWidth = 40, MinHeight = 60,
							Id = "the-tsp", 
							Style = "background-color: dark",
							Map = luxembourg,
							Decay = TSP.getDecay(DEFAULT_DECAY)
						}
					}
				}
			}
		}
	}
}

--	Add the solver as a coroutine. This way the GUI remains reactive:

app:addCoroutine(function(self)
	local tsp = self:getById("the-tsp")
	while true do
		self:suspend()
		tsp:opt()
	end
end, app)

--	run the application:

app:run()

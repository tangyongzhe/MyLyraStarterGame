import { Init } from './Mono/Init';
import UE = require('ue');
import { argv, toDelegate } from 'puerts';
import { Define } from './Mono/Define';
import { ToMinix } from "./AutoMixin";
//import './Test/puerts-loadue-type-compile';

Define.Game = argv.getByName("GameInstance") as UE.LyraGameInstance;
//ToMinix(Define.Game, "AutoMixin");
Define.Game.BindMixin(toDelegate(Define.Game, ToMinix));

Init.start();

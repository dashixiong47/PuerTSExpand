import * as UE from 'ue';
import { blueprint } from 'puerts';

let ucls = UE.Class.Load("<FullObjectPath>_C")
let jsCls = blueprint.tojs<typeof UE<AssetTypes>_C>(ucls);
interface <AssetName> extends UE<AssetTypes>_C { }

class <AssetName> implements <AssetName> {
    
}
blueprint.mixin(jsCls, <AssetName>);

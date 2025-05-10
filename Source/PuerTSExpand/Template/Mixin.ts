import * as UE from 'ue';
import { blueprint } from 'puerts';

let ucls = UE.Class.Load("<FullObjectPath>_C")
let jsCls = blueprint.tojs<typeof <AssetTypes>>(ucls);
interface <AssetName> extends <AssetTypes> { }

class <AssetName> implements <AssetName> {
    
}
blueprint.mixin(jsCls, <AssetName>);

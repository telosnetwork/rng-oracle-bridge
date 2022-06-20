module.exports = async ({getNamedAccounts, deployments}) => {
    const {deploy} = deployments;
    const {deployer} = await getNamedAccounts();


    const bridge = await deploy('NativeOracleRequest', {
        from: deployer,
        args: [],
    });

    console.log("Deployed to:", bridge.address);

};
module.exports.tags = ['NativeOracleRequest'];